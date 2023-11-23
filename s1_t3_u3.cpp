#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

/*bool CompareStrings(const string& s1, const string& s2) {
    return lexicographical_compare(
        s1.begin(), s1.end(), // Первый диапазон
        s2.begin(), s2.end(), // Второй диапазон
        [](char left, char right) {
            // Сравниваем символы left и right без учёта их регистра.
            // Возвращаем true, если left предшествует right.
            return tolower(static_cast<unsigned char>(left)) < tolower(static_cast<unsigned char>(right));
        }
    );
}*/

int main()
{
    // считайте входные данные и сформируйте вывод программы
    // ...
    string str, tmp_str;
    vector<string> vec_str;
    int n = 0, j;
    int idx = 4;
    int len = 0;

    getline(cin, str);
    len = str.size();

    for (int i = 0; i < 4; ++i)
    {
        if (str[i] != ' ')
        {
            n *= 10;
            n += static_cast<int>(str[i] - '0');
        }
        else
        {
            idx = i + 1; // запоминаем позицию после пробела
        }
    }

    for (int i = 0; i < n; ++i)
    {
        if (idx >= len)
        { // нельзя выходить за границы строки
            break;
        }
        for (j = 0; j < 15; ++j)
        {
            if (j + idx >= len)
            { // нельзя выходить за границы строки
                break;
            }
            if (str[j + idx] != ' ')
            {
                tmp_str += str[j + idx];
            }
            else
            {
                break;
            }
        }
        idx += (j + 1); // запоминаем позицию после пробела
        vec_str.push_back(tmp_str);
        tmp_str = ""s;
    }

    // sort(vec_str.begin(), vec_str.end(), CompareStrings);
    sort(vec_str.begin(), vec_str.end(), [](const string &s1, const string &s2) -> bool
         { return lexicographical_compare(
               s1.begin(), s1.end(), // Первый диапазон
               s2.begin(), s2.end(), // Второй диапазон
               [](char left, char right)
               {
                   // Сравниваем символы left и right без учёта их регистра.
                   // Возвращаем true, если left предшествует right.
                   return tolower(static_cast<unsigned char>(left)) < tolower(static_cast<unsigned char>(right));
               }); });

    for (string s : vec_str)
    {
        cout << s << " "s;
    }
    cout << endl;

    return 0;
}