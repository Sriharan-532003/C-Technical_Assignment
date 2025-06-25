#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

// Global variables for thread safety and control
std::mutex frameMutex;
cv::Mat currentFrame;
std::atomic<bool> isRunning(true);

// Thread-safe flag to save snapshot
std::atomic<bool> takeSnapshot(false);

// Thread to continuously capture frames from the webcam
void frameCapture(cv::VideoCapture& camera) 
{
    while (isRunning) 
    {
        cv::Mat frame;
        camera >> frame;
        if (frame.empty()) break;

        // Update shared frame safely
        {
            std::lock_guard<std::mutex> lock(frameMutex);
            currentFrame = frame.clone();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Thread to continuously write frames to a video file
void videoRecorder(cv::VideoWriter& writer) 
{
    while (isRunning) 
    {
        cv::Mat frameToWrite;

        {
            std::lock_guard<std::mutex> lock(frameMutex);
            if (!currentFrame.empty()) 
	    {
                frameToWrite = currentFrame.clone();
            }
        }

        if (!frameToWrite.empty()) 
	{
            writer.write(frameToWrite);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

// Function to save a snapshot from the latest frame
void saveSnapshot() 
{
    static int snapshotCount = 0;

    cv::Mat snapshot;
    {
        std::lock_guard<std::mutex> lock(frameMutex);
        snapshot = currentFrame.clone();
    }

    if (!snapshot.empty()) 
    {
        std::string filename = "snapshot_" + std::to_string(snapshotCount++) + ".jpg";
        if (cv::imwrite(filename, snapshot)) 
	{
            std::cout << "Snapshot saved: " << filename << "\n";
        } else 
	{
            std::cerr << "Failed to save snapshot.\n";
        }
    }
}

int main() 
{
    // Open the default webcam (device 0)
    cv::VideoCapture camera(0);
    if (!camera.isOpened()) 
    {
        std::cerr << "Error: Unable to open the camera.\n";
        return -1;
    }

    // Get frame dimensions
    int width = static_cast<int>(camera.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(camera.get(cv::CAP_PROP_FRAME_HEIGHT));

    // Initialize the video writer to save the live stream
    cv::VideoWriter writer("output.avi",
                           cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                           20,
                           cv::Size(width, height));

    // Launch threads for capturing and writing video
    std::thread captureThread(frameCapture, std::ref(camera));
    std::thread recordingThread(videoRecorder, std::ref(writer));

    std::cout << "Live Video Capture Started\n";
    std::cout << "Press 's' to take a snapshot, 'q' to quit.\n";

    // Main display loop
    while (isRunning) 
    {
        cv::Mat frameToDisplay;

        {
            std::lock_guard<std::mutex> lock(frameMutex);
            if (!currentFrame.empty()) 
	    {
                frameToDisplay = currentFrame.clone();
            }
        }

        if (!frameToDisplay.empty()) 
	{
            cv::imshow("Live Feed", frameToDisplay);
        }

        char key = static_cast<char>(cv::waitKey(1));
        if (key == 'q') 
	{
            isRunning = false;
        } else if (key == 's') 
	{
            std::thread(saveSnapshot).detach(); // Save snapshot in separate thread
        }
    }

    // Clean up and exit
    captureThread.join();
    recordingThread.join();
    camera.release();
    writer.release();
    cv::destroyAllWindows();

    std::cout << "Application exited cleanly.\n";
    return 0;
}

