#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// Constants
const std::string BASE_DIRECTORY = "./data/";
const char ENCRYPTION_KEY = 0xAA; // Simple XOR-based encryption key

// XOR encryption/decryption function
std::string xorTransform(const std::string& input) 
{
    std::string result = input;
    for (char& character : result) 
    {
        character ^= ENCRYPTION_KEY;
    }
    return result;
}

// Function to store encrypted information in a file
void storeEncryptedFile() 
{
    std::string country, state, district, description;

    std::cout << "\nEnter the following details:\n";
    std::cout << "Country: ";
    std::getline(std::cin, country);
    std::cout << "State: ";
    std::getline(std::cin, state);
    std::cout << "District: ";
    std::getline(std::cin, district);
    std::cout << "Description: ";
    std::getline(std::cin, description);

    // Build directory path based on location hierarchy
    fs::path fullPath = fs::path(BASE_DIRECTORY) / country / state / district;

    try 
    {
        // Create the directory structure if it doesn't exist
        fs::create_directories(fullPath);

        // Format the input content
        std::string content = 
            "Country: " + country + "\n" +
            "State: " + state + "\n" +
            "District: " + district + "\n" +
            "Description: " + description + "\n";

        // Encrypt the content
        std::string encryptedContent = xorTransform(content);
        fs::path filePath = fullPath / "info.dat";

        // Write the encrypted content to a binary file
        std::ofstream outputFile(filePath, std::ios::binary);
        outputFile << encryptedContent;
        outputFile.close();

        std::cout << "File successfully saved at: " << filePath << "\n";
    } catch (const std::exception& error) 
    {
        std::cerr << "Error: " << error.what() << "\n";
    }
}

// Function to retrieve and decrypt a file
void retrieveDecryptedFile() 
{
    std::string country, state, district;

    std::cout << "\nEnter location to retrieve file:\n";
    std::cout << "Country: ";
    std::getline(std::cin, country);
    std::cout << "State: ";
    std::getline(std::cin, state);
    std::cout << "District: ";
    std::getline(std::cin, district);

    fs::path filePath = fs::path(BASE_DIRECTORY) / country / state / district / "info.dat";

    // Check if the encrypted file exists
    if (!fs::exists(filePath)) 
    {
        std::cerr << "⚠️ File not found at: " << filePath << "\n";
        return;
    }

    try 
    {
        // Read the encrypted binary file
        std::ifstream inputFile(filePath, std::ios::binary);
        std::string encryptedContent((std::istreambuf_iterator<char>(inputFile)),
                                      std::istreambuf_iterator<char>());
        inputFile.close();

        // Decrypt and display the content
        std::string decryptedContent = xorTransform(encryptedContent);
        std::cout << "\nDecrypted File Content:\n";
        std::cout << "------------------------------\n";
        std::cout << decryptedContent;
        std::cout << "------------------------------\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
    }
}

// Main menu-driven interface
int main() 
{
    while (true) 
    {
        std::cout << "\n====== Encrypted File Manager ======\n";
        std::cout << "1. Store a new encrypted file\n";
        std::cout << "2. Retrieve and view a decrypted file\n";
        std::cout << "3. Exit\n";
        std::cout << "Choose an option (1-3): ";

        int choice;
        std::cin >> choice;
        std::cin.ignore(); // Clear newline from input buffer

        switch (choice) 
	{
            case 1:
                storeEncryptedFile();
                break;
            case 2:
                retrieveDecryptedFile();
                break;
            case 3:
                std::cout << "Exiting the program.\n";
                return 0;
            default:
                std::cerr << "Invalid option. Please try again.\n";
        }
    }
}

