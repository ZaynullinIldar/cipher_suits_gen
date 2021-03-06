#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

struct token
{
    std::string cipher;
    std::string cipher_name;
};

using tokens = std::vector<token>;
using record = std::string;
using records = std::vector<record>;

std::string get_cipher(const std::string& raw_cipher)
{
    std::string result;
    result.resize(6);
    result[0] = raw_cipher[0];
    result[1] = raw_cipher[1];
    result[2] = raw_cipher[2];
    result[3] = raw_cipher[3];
    result[4] = raw_cipher[7];
    result[5] = raw_cipher[8];
    return result;
}

std::string copy(const std::string& record, int first, int last)
{
    std::string result;
    for (int i = first ;i < last; ++i)
    {
        result.push_back(record[i]);
    }
    return result;
}

bool check_cipher(const std::string& cipher)
{
    for (int i = 2; i <= 5; ++i)
    {
        if (!isalpha(cipher[i]) && !isdigit(cipher[i]))
        {
            return false;
        }
    }
    return true;
}

bool parse_record(const std::string& record, token& result)
{
    std::string raw_cipher;
    std::string cipher;
    std::string cipher_name;
    auto first = record.find('"', 0);
    auto last = record.find('"', 1);
    if (first != record::npos && last != record::npos)
    {
        raw_cipher.resize(9);
        raw_cipher = copy(record, first + 1, last);
        cipher = get_cipher(raw_cipher);
         if (check_cipher(cipher))
         {
             first = record.find("TLS");
             last = record.find("RFC");
             if (first != record::npos && last != record::npos)
             {
                 result.cipher = cipher;
                 cipher_name = copy(record, first, last - 6);
                 result.cipher_name = cipher_name;
                 return true;
             }
         }
    }
    return false;
}

tokens get_tokens(const records& data)
{
    tokens result;
    token temp;
    for (auto& item : data)
    {
       if (parse_record(item, temp))
       {
           result.push_back(temp);
       }
    }
    return result;
}

records get_records(const std::string& data)
{
    records result;
    record temp;
    for (auto& item : data)
    {
        if (item != '\n')
        {
            temp.push_back(item);
        }
        else
        {
            result.push_back(temp);
            temp.clear();
        }
    }
    return result;
}

std::string gen_cpp_func(const tokens& data)
{
    std::ostringstream out;
    out << '\n';
    out << '\n';
    out << "// this function generated by tls_cipher_suits_gen utility" << '\n';
    out << '\n';
    out << "std::string get_str_tls_cipher_suits(size_t cipher);" << '\n';
    out << '\n';
    out << "std::string" << '\n';
    out << "get_str_tls_cipher_suits(size_t cipher) {" << '\n';
    out << "    switch(cipher) {" << '\n';
    for (auto& item : data)
    {
        out << "    case " << item.cipher <<": return " << "\"" << item.cipher_name << "\"" << ";" << '\n';
    }
    out << "    }" << '\n';
    out << "    return " << "\"" << "UNKNOWN" << "\"" << ";" << '\n';
    out << "}" << '\n';
    return out.str();
}

int main(int argc, char* argv[])
{
    if (argc > 2 || argc == 1)
    {
        std::cerr << "Usage: tls_cipher_suits_gen [name of file *.csv]" << std::endl;
        return -1;
    }

    std::string name_file{argv[1]};
    std::ifstream in;
    in.open(name_file);

    if (!in)
    {
        std::cerr << "Couldn't open file " << name_file << std::endl;
        return -2;
    }

    std::stringstream buf{};
    buf << in.rdbuf();
    in.close();

    std::string data{buf.str()};

    auto records_data = get_records(data);
    auto tokens_data = get_tokens(records_data);

    auto cpp_func = gen_cpp_func(tokens_data);
    std::ofstream out;
    out.open("tls_cipher_suits.cc");
    out << cpp_func;
    out.close();
    return 0;
}