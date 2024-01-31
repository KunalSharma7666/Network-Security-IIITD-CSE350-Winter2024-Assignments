#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <fstream>
using namespace std;
string hashfunction(const string &data)
{
    const uint32_t polynomial = 0xEDB88320;
    uint32_t crc = 0xFFFFFFFF;
    int x = 0;
    while (x < data.size())
    {
        crc ^= static_cast<uint32_t>(data[x]);
        int i = 0;
        while (i < 8)
        {
            crc = (crc >> 1) ^ ((crc & 1) ? polynomial : 0);
            i++;
        }
        x++;
    }
    crc ^= 0xFFFFFFFF;
    string result;
    while (crc > 0)
    {
        uint32_t temp = static_cast<char>(crc % 26);
        result.insert(result.begin(), 'a' + temp);
        crc /= 26;
    }
    if (result.length() < 8)
    {
        while (result.length() < 8)
        {
            result.insert(result.begin(), 'a');
        }
    }
    if (result.length() > 8)
    {
        result = result.substr(0, 8);
    }
    return result;
}
string encrypt(string plaintext, string key)
{
    vector<vector<char> > v;
    int key_len = key.length();
    plaintext += hashfunction(plaintext);
    float temp1 = (float)plaintext.size() / (float)key_len;
    int num_rows = ceil(temp1);
    int x = 0;
    for (int i = 0; i < num_rows; i++)
    {
        vector<char> temp;
        for (int j = 0; j < key_len; j++)
        {
            if (x < plaintext.length())
            {
                temp.push_back(plaintext[x]);
                x++;
            }
            else
            {
                temp.push_back('-');
            }
        }
        v.push_back(temp);
    }
    char c = '0';
    string ciphertext(key_len * num_rows, c);
    for (int i = 0; i < key_len; i++)
    {
        for (int j = 0; j < num_rows; j++)
        {
            ciphertext[(num_rows * ((key[i] - '0') - 1)) + j] = v[j][i];
        }
    }
    return ciphertext;
}

string decrypt(string ciphertext, string key)
{
    int key_len = key.length();
    float temp1 = ciphertext.length() / key_len;
    int num_rows = ceil(temp1);
    vector<vector<char> > v(num_rows, vector<char>(key_len, '0'));
    for (int i = 0; i < key_len; i++)
    {
        for (int j = 0; j < num_rows; j++)
        {
            v[j][i] = ciphertext[(num_rows * ((key[i] - '0') - 1)) + j];
        }
    }
    char c = '0';
    string plaintext = "";
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < key_len; j++)
        {
            plaintext += v[i][j];
        }
    }
    return plaintext;
}

string removechar(string &word, char &ch)
{
    for (int i = 0; i < word.length(); i++)
    {
        if (word[i] == ch)
        {
            word.erase(word.begin() + i);
            i--;
        }
    }

    return word;
}

void string_permutation(vector<string> cipher_strings, string &orig, string &perm, string &ans)
{
    if (orig.empty())
    {
        char ch = '-';
        string temp_plaintext = decrypt(cipher_strings[0], perm);
        removechar(temp_plaintext, ch);
        if (hashfunction(temp_plaintext.substr(0, temp_plaintext.length() - 8)) == temp_plaintext.substr(temp_plaintext.length() - 8, 8))
        {
            string temp_plaintext2 = decrypt(cipher_strings[1], perm);
            removechar(temp_plaintext2, ch);
            if (hashfunction(temp_plaintext2.substr(0, temp_plaintext2.length() - 8)) == temp_plaintext2.substr(temp_plaintext2.length() - 8, 8))
            {
                string temp_plaintext3 = decrypt(cipher_strings[2], perm);
                removechar(temp_plaintext3, ch);
                if (hashfunction(temp_plaintext3.substr(0, temp_plaintext3.length() - 8)) == temp_plaintext3.substr(temp_plaintext3.length() - 8, 8))
                {
                    string temp_plaintext4 = decrypt(cipher_strings[3], perm);
                    removechar(temp_plaintext4, ch);
                    if (hashfunction(temp_plaintext4.substr(0, temp_plaintext4.length() - 8)) == temp_plaintext4.substr(temp_plaintext4.length() - 8, 8))
                    {
                        string temp_plaintext5 = decrypt(cipher_strings[4], perm);
                        removechar(temp_plaintext5, ch);
                        if (hashfunction(temp_plaintext5.substr(0, temp_plaintext5.length() - 8)) == temp_plaintext5.substr(temp_plaintext5.length() - 8, 8))
                        {
                            ans = perm;
                        }
                    }
                }
            }
        }
        return;
    }

    for (int i = 0; i < orig.size(); ++i)
    {
        string orig2 = orig;
        orig2.erase(i, 1);
        string perm2 = perm;
        perm2 += orig.at(i);
        string_permutation(cipher_strings, orig2, perm2, ans);
    }
}

string bruteforceattack(vector<string> cipher_strings)
{
    string ans;
    for (int j = 1; j <= 9; j++)
    {
        string key;
        for (int i = 0; i < j; i++)
        {
            key += to_string(i + 1);
        }
        string perm;
        string temp;
        string_permutation(cipher_strings, key, perm, ans);
    }
    return ans;
}
int main()
{
    ifstream myfile("key.txt");
    ifstream myfile2("input.txt");
    vector<string> input_strings(5);
    if (myfile2.is_open())
    {
        int i = 0;
        while (myfile2)
        {
            getline(myfile2, input_strings[i]);
            i++;
        }
    }
    string key;
    if (myfile.is_open())
    {
        while (myfile)
        {
            getline(myfile, key);
        }
    }
    vector<string> cipher_strings;
    for (int i = 0; i < 5; i++)
    {
        cipher_strings.push_back(encrypt(input_strings[i], key));
    }
    cout<< "Brute force attack in progress...."<< endl;
    cout << "Key found after Brute force attack: "<< bruteforceattack(cipher_strings);
}