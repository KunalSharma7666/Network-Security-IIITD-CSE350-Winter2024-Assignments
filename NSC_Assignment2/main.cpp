#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>
#include <random>
#include <iomanip>
#include <sstream>
#include <fstream>
using namespace std;

void rotate(vector<uint8_t> &vec, int d)
{
    if (d == 0)
        return;
    for (int i = 0; i < d; i++)
    {
        vec.push_back(vec[0]);
        vec.erase(vec.begin());
    }
}

vector<int> matrixMultiplicationAndAddition(const int matrixA[][8], const int matrixB[], const vector<int> &matrixC)
{
    vector<int> result(8, 0);

    // Matrix multiplication
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            result[i] ^= matrixA[i][j] & matrixC[j];
        }
    }

    // Matrix addition
    for (int i = 0; i < 8; ++i)
    {
        result[i] ^= matrixB[i];
    }

    return result;
}

uint8_t gf_multiply(uint8_t a, uint8_t b)
{
    uint8_t result = 0;
    uint8_t carry;

    for (int i = 0; i < 8; i++)
    {
        if (b & 1)
            result ^= a;

        carry = a & 0x80;

        a <<= 1;
        if (carry)
            a ^= 0x1B;

        b >>= 1;
    }
    return result;
}

uint8_t m_inv(uint8_t x)
{
    if (x == 0x00)
        return 0x00;
    for (uint8_t i = 0x01; i != 0x00; i++)
    {
        if (gf_multiply(x, i) == 0x01)
        {
            return i;
        }
    }
    return 0x00;
}

uint8_t substitution(uint8_t num)
{
    uint8_t temp = m_inv(num);
    // cout<<hex<<(int)temp;
    int matrixA[8][8] = {
        {1, 0, 0, 0, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 1, 1, 1},
        {1, 1, 1, 0, 0, 0, 1, 1},
        {1, 1, 1, 1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 0, 0, 0},
        {0, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 1, 1, 1, 1, 1}};

    int matrixB[] = {1, 1, 0, 0, 0, 1, 1, 0};
    vector<int> vec(8);
    auto x = bitset<8>(temp);
    for (int i = 7; i >= 0; i--)
    {
        vec[7 - i] = x[i];
    }
    vec = matrixMultiplicationAndAddition(matrixA, matrixB, vec);
    for (int i = 7; i >= 0; i--)
    {
        x[i] = vec[7 - i];
    }
    temp = int(x.to_ulong());
    temp = static_cast<u_int8_t>(temp);
    return temp;
}

uint8_t inv_substitution(uint8_t num)
{
    int matrixA[8][8] = {
        {0, 0, 1, 0, 0, 1, 0, 1},
        {1, 0, 0, 1, 0, 0, 1, 0},
        {0, 1, 0, 0, 1, 0, 0, 1},
        {1, 0, 1, 0, 0, 1, 0, 0},
        {0, 1, 0, 1, 0, 0, 1, 0},
        {0, 0, 1, 0, 1, 0, 0, 1},
        {1, 0, 0, 1, 0, 1, 0, 0},
        {0, 1, 0, 0, 1, 0, 1, 0}};

    int matrixB[] = {1, 0, 1, 0, 0, 0, 0, 0};
    vector<int> vec(8);
    auto x = bitset<8>(num);
    for (int i = 7; i >= 0; i--)
    {
        vec[7 - i] = x[i];
    }
    vec = matrixMultiplicationAndAddition(matrixA, matrixB, vec);
    for (int i = 7; i >= 0; i--)
    {
        x[i] = vec[7 - i];
    }
    uint8_t temp = int(x.to_ulong());
    temp = static_cast<u_int8_t>(temp);
    temp = m_inv(temp);

    return temp;
}

uint8_t RC(int x)
{
    if (x == 0x01)
        return 0x01;
    return gf_multiply(0x02, RC(x - 1));
}

void g_func(vector<uint8_t> &vec, int round_number)
{
    rotate(vec, 1);
    for (int i = 0; i < 4; i++)
    {
        vec[i] = substitution(vec[i]);
    }
    vec[0] = vec[0] ^ RC(round_number);
}

void create_round_key(vector<vector<uint8_t>> &prev_round_key, int round_number)
{
    vector<uint8_t> vec;
    for (int i = 0; i < 4; i++)
    {
        vec.push_back(prev_round_key[i][3]);
    }
    g_func(vec, round_number);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == 0)
            {
                prev_round_key[j][i] ^= vec[j];
            }
            else
            {
                prev_round_key[j][i] ^= prev_round_key[j][i - 1];
            }
        }
    }
}

void inv_create_round_key(vector<vector<uint8_t>> &prev_round_key, int round_number)
{
    if (round_number == 1)
    {
        return;
    }
    vector<uint8_t> vec;
    for (int i = 3; i >= 0; i--)
    {
        for (int j = 3; j >= 0; j--)
        {
            if (i == 0)
            {
                prev_round_key[j][i] ^= vec[j];
            }
            else
            {
                prev_round_key[j][i] ^= prev_round_key[j][i - 1];
            }
        }
        if (i == 3)
        {
            for (int k = 0; k < 4; k++)
            {
                vec.push_back(prev_round_key[k][3]);
            }
            g_func(vec, 12 - round_number);
        }
    }
}

vector<vector<uint8_t>> encrypt_round(vector<vector<uint8_t>> plaintext_matrix_hex, vector<vector<uint8_t>> ciphertext_matrix_hex, int round_number, vector<vector<uint8_t>> &prev_round_key)
{
    // Substitution
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ciphertext_matrix_hex[i][j] = substitution(plaintext_matrix_hex[i][j]);
        }
    }
    // Shift Rows
    for (int i = 0; i < 4; i++)
    {
        rotate(ciphertext_matrix_hex[i], i);
    }

    if (round_number != 10)
    {
        // Mix Columns
        uint8_t matrix[4][4] = {
            {0x02, 0x03, 0x01, 0x01},
            {0x01, 0x02, 0x03, 0x01},
            {0x01, 0x01, 0x02, 0x03},
            {0x03, 0x01, 0x01, 0x02}};
        vector<vector<uint8_t>> ciphertext_m_temp(4, vector<uint8_t>(4));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                uint8_t ans = 0x00;
                for (int k = 0; k < 4; k++)
                {
                    ans ^= gf_multiply(matrix[i][k], ciphertext_matrix_hex[k][j]);
                }
                ciphertext_m_temp[i][j] = ans;
            }
        }
        // Add Round Key
        create_round_key(prev_round_key, round_number);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                ciphertext_m_temp[i][j] ^= prev_round_key[i][j];
            }
        }
        return ciphertext_m_temp;
    }
    // Add Round Key
    create_round_key(prev_round_key, round_number);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ciphertext_matrix_hex[i][j] ^= prev_round_key[i][j];
        }
    }

    return ciphertext_matrix_hex;
}

vector<vector<uint8_t>> decrypt_round(vector<vector<uint8_t>> plaintext_matrix_hex, vector<vector<uint8_t>> ciphertext_matrix_hex, int round_number, vector<vector<uint8_t>> &prev_round_key)
{
    // Add Round Key
    inv_create_round_key(prev_round_key, round_number);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ciphertext_matrix_hex[i][j] ^= prev_round_key[i][j];
        }
    }
    if (round_number != 1)
    {
        // Inverse Mix Columns
        uint8_t matrix[4][4] = {
            {0x0e, 0x0b, 0x0d, 0x09},
            {0x09, 0x0e, 0x0b, 0x0d},
            {0x0d, 0x09, 0x0e, 0x0b},
            {0x0b, 0x0d, 0x09, 0x0e}};
        vector<vector<uint8_t>> ciphertext_m_temp(4, vector<uint8_t>(4));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                uint8_t ans = 0x00;
                for (int k = 0; k < 4; k++)
                {
                    ans ^= gf_multiply(matrix[i][k], ciphertext_matrix_hex[k][j]);
                }
                ciphertext_m_temp[i][j] = ans;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                ciphertext_matrix_hex[i][j] = ciphertext_m_temp[i][j];
            }
        }
    }
    // Inverse Shift Rows
    for (int i = 4; i > 0; i--)
    {
        rotate(ciphertext_matrix_hex[4 - i], i);
    }
    // Inverse Substitution
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            plaintext_matrix_hex[i][j] = inv_substitution(ciphertext_matrix_hex[i][j]);
        }
    }

    return plaintext_matrix_hex;
}

vector<vector<uint8_t>> encrypt(vector<vector<string>> plaintext_matrix_hex, vector<vector<uint8_t>> &ciphertext_matrix_hex, int round_number, vector<vector<uint8_t>> &key)
{
    // Initial Add Round
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            unsigned long l = stoul(plaintext_matrix_hex[i][j], nullptr, 16);
            uint8_t x = static_cast<uint8_t>(l);
            ciphertext_matrix_hex[i][j] = x ^ key[i][j];
        }
    }
    // Next 10 rounds
    vector<vector<uint8_t>> temp(4, vector<uint8_t>(4));
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            temp[i][j] = ciphertext_matrix_hex[i][j];
        }
    }
    for (int i = 1; i <= 10; i++)
    {
        temp = encrypt_round(temp, temp, i, key);
        if (i == 1)
        {
            cout << "Output of 1st encryption round: ";
            for (int j = 0; j < 4; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    cout << hex << (int)temp[k][j];
                }
            }
            cout << "\n";
        }
        if (i == 9)
        {
            cout << "Output of 9th encryption round: ";
            for (int j = 0; j < 4; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    cout << hex << (int)temp[k][j];
                }
            }
            cout << "\n";
        }
    }
    return temp;
}

vector<vector<uint8_t>> decrypt(vector<vector<string>> ciphertext_matrix_hex, vector<vector<uint8_t>> &plaintext_matrix_hex, int round_number, vector<vector<uint8_t>> &key)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            unsigned long l = stoul(ciphertext_matrix_hex[i][j], nullptr, 16);
            uint8_t x = static_cast<uint8_t>(l);
            plaintext_matrix_hex[i][j] = x;
        }
    }
    // Next 10 rounds
    vector<vector<uint8_t>> temp(4, vector<uint8_t>(4));
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            temp[i][j] = plaintext_matrix_hex[i][j];
        }
    }
    for (int i = 1; i <= 10; i++)
    {
        temp = decrypt_round(temp, temp, i, key);
        if (i == 1)
        {
            cout << "Output of 1st decryption round: ";
            for (int j = 0; j < 4; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    cout << hex << (int)temp[k][j];
                }
            }
            cout << "\n";
        }
        if (i == 9)
        {
            cout << "Output of 9th decryption round: ";
            for (int j = 0; j < 4; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    cout << hex << (int)temp[k][j];
                }
            }
            cout << "\n";
        }
    }
    // Last Add Round
    inv_create_round_key(key, 11);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            temp[i][j] ^= key[i][j];
        }
    }
    return temp;
}

int main()
{
    ifstream myfile("input.txt");
    vector<string> plaintext(5);
    if (myfile.is_open())
    {
        int i = 0;
        while (myfile)
        {
            getline(myfile, plaintext[i]);
            i++;
        }
    }
    for (int loop = 0; loop < 3; loop++)
    {
        cout << "Initial Plaintext: " << plaintext[loop] << "\n";
        vector<vector<string>> plaintext_matrix(4, vector<string>(4));

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                string temp = "";
                for (int k = 0; k < 8; k++)
                {
                    temp += to_string((plaintext[loop][(4 * i) + j] >> k) & 1);
                }
                reverse(temp.begin(), temp.end());
                plaintext_matrix[j][i] = temp;
            }
        }
        vector<vector<string>> plaintext_matrix_hex(4, vector<string>(4));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                for (int x = 0; x < 2; x++)
                {
                    string hexn = plaintext_matrix[i][j].substr(x * 4, 4);
                    int num = 0;
                    for (int k = 3; k >= 0; k--)
                    {
                        if (hexn[k] == '1')
                        {
                            num += pow(2, abs(k - 3));
                        }
                    }
                    if (num <= 9)
                    {
                        plaintext_matrix_hex[i][j] += to_string(num);
                    }
                    else
                    {
                        plaintext_matrix_hex[i][j] += 'A' + num - 10;
                    }
                }
            }
        }

        vector<vector<string>> sbox1(16, std::vector<string>(16));

        vector<vector<uint8_t>> key{
            {0xea, 0x2d, 0x82, 0x7f},
            {0xd2, 0x37, 0x34, 0x8d},
            {0x73, 0xbf, 0x5c, 0x29},
            {0x21, 0x09, 0xa8, 0x2f}};

        vector<vector<uint8_t>> ciphertext_matrix_hex(4, vector<uint8_t>(4));
        vector<vector<uint8_t>> ans = encrypt(plaintext_matrix_hex, ciphertext_matrix_hex, 0, key);
        cout << "Ciphertext: ";
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                cout << hex << (int)ans[i][j];
            }
        }
        cout << "\n";
        vector<vector<string>> cipher(4, vector<string>(4));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                stringstream ss;
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ans[i][j]);
                cipher[i][j] = ss.str();
            }
        }
        ans = decrypt(cipher, ciphertext_matrix_hex, 0, key);
        cout << "Final Plaintext: ";
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                std::bitset<8> binaryValue(ans[j][i]);
                char asciiChar = static_cast<char>(binaryValue.to_ulong());
                cout << asciiChar;
            }
        }
        cout << "\n"
             << "\n";
    }
    return 0;
}