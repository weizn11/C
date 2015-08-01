#include <iostream>
#include <string>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
using namespace std;
class Operation
{
public:
	char Check_Oper(string &str, string &num_1, string &num_2);     //检测运算符
	string addition(string &num1, string &num2);                   //加法运算
	string subduction(string &num1, string &num2);                 //减法运算
	string multiplication(string &num1, string &num2);             //乘法运算
	string division(string &num1, string &num2);                   //除法运算
	string factorial(string &num);                                //阶乘运算
private:
	string Reverse_String(string &str);                           //反转字符串
	string SUB_ASC(string &str);                                  //字符串ASCII码减48位
	string ADD_ASC(string &str);                                  //字符串ASCII码加48位
	string GET_MAX(string &num1, string &num2);                    //获取两个数中的最大值
};
string Operation::addition(string &num1, string &num2)
{
	unsigned short a = 0, b = 0;
	string result;
	string num_1(num1);
	string num_2(num2);
	string::size_type i = 0, j = 0;
	Reverse_String(num_1);
	Reverse_String(num_2);
	SUB_ASC(num_1);
	SUB_ASC(num_2);
	for (i = 0; i != num_1.size(); i++) a += num_1[i];
	for (j = 0; j != num_2.size(); j++) b += num_2[j];
	if (a != 0 || b != 0) goto skip;
	result.insert(result.begin(), '0');
	return result;
skip:
	i = j = 0;
	while (i != num_1.size() || j != num_2.size())
	{
		if (i != num_1.size() && j != num_2.size()) result.insert(result.end(), num_1[i] + num_2[j]);
		else if (i != num_1.size()) result.insert(result.end(), num_1[i]);
		else if (j != num_2.size()) result.insert(result.end(), num_2[j]);
		i == num_1.size() ? i : i++;
		j == num_2.size() ? j : j++;
	}
	for (i = 0; i != result.size(); i++)
	if (result[i]>9)
	{
		result[i] %= 10;
		if (i + 1 == result.size()) result.insert(result.end(), 0);
		result[i + 1] += 1;
	}
	ADD_ASC(result);
	Reverse_String(result);
	while (1)
	{
		if (*result.begin() == '0') result.erase(result.begin());
		else break;
	}
	return result;
}
string Operation::subduction(string &num1, string &num2)
{
	short k = 0;
	string result;
	string::size_type i = 0, j = 0;
	string num_1(num1);
	string num_2(num2);
	if (GET_MAX(num_1, num_2) == num_2)
	{
		k = 1;
		num_2.swap(num_1);       //交换对象数据
	}
	Reverse_String(num_1);
	Reverse_String(num_2);
	SUB_ASC(num_1);
	SUB_ASC(num_2);
	i = j = 0;
	while (i != num_1.size() || j != num_2.size())
	{
		if (i != num_1.size() && j != num_2.size()) result.insert(result.end(), num_1[i] - num_2[j]);
		else if (i != num_1.size()) result.insert(result.end(), num_1[i]);
		else if (j != num_2.size()) result.insert(result.end(), num_2[j]);
		i == num_1.size() ? i : i++;
		j == num_2.size() ? j : j++;
	}
	for (i = 0; i != result.size(); i++)
	if (result[i]<0)
	{
		result[i] = 10 + result[i];
		result[i + 1] -= 1;
	}
	ADD_ASC(result);
	Reverse_String(result);
	while (1)
	{
		if (*result.begin() == '0') result.erase(result.begin());
		else break;
	}
	if (k)
	{
		if (result.empty())
			result.insert(result.end(), '0');
		else
			result.insert(result.begin(), '-');
	}
	return result;
}
string Operation::multiplication(string &num1, string &num2)
{
	string result, temp;
	string num_1(num1);
	string num_2(num2);
	string::size_type i = 0, j = 0, m = 0;
	short k = 1;
	char tmp = NULL;
	Reverse_String(num_1);
	Reverse_String(num_2);
	SUB_ASC(num_1);
	SUB_ASC(num_2);
	for (i = 0; i != num_1.size(); i++) result.insert(result.end(), num_2[0] * num_1[i]);
	for (m = 0; m != result.size(); m++)
	if (result[m]>9)
	{
		tmp = result[m];
		result[m] %= 10;
		if (m + 1 == result.size()) result.insert(result.end(), 0);
		result[m + 1] += tmp / 10;
	}
	while (j != num_2.size() - 1)
	{
		j++;
		if (j == num_2.size()) break;
		for (i = 0; i != num_1.size(); i++) temp.insert(temp.end(), num_2[j] * num_1[i]);
		temp.insert(temp.begin(), k, 0);
		for (m = 0; m<temp.size(); m++) temp[m] += result[m];
		for (m = 0; m<temp.size(); m++)
		if (temp[m]>9)
		{
			tmp = temp[m];
			temp[m] %= 10;
			if (m + 1 == temp.size()) temp.insert(temp.end(), 0);
			temp[m + 1] += tmp / 10;
		}
		result.assign(temp);
		ADD_ASC(result);
		SUB_ASC(result);
		temp.erase(temp.begin(), temp.end());
		k++;
	}
	Reverse_String(result);
	ADD_ASC(result);
	return result;
}
string Operation::division(string &num1, string &num2)
{
	string result;
	string MAX;
	string remainder;
	string num1_tmp;
	string num_1(num1);
	string num_2(num2);
	string step(1, '1');
	string temp1, temp2;
	string::iterator iter;
	string::size_type i = 0;
	while (*num_2.begin() == '0') num_2.erase(num_2.begin());
	if (num_2.empty())
	{
		result = "除数不可以为零!";
		return result;
	}
	num_1.insert(num_1.end(), 100, '0');
	iter = num_1.begin() + num_2.size();
	num1_tmp.assign(num_1.begin(), iter);
	iter--;
again:
	for (step[0] = '1'; 1; step[0]++)
	{
		MAX.assign(GET_MAX(multiplication(num_2, step), num1_tmp));
		if (step[0] == '9' && num1_tmp == MAX) goto skip;
		if (MAX != num1_tmp)
		{
			step[0]--;
		skip:
			temp2.assign(subduction(num1_tmp, multiplication(num_2, step)));
			while (1)
			if (temp2[0] == '0') temp2.erase(temp2.begin());
			else break;
			result += step;
			iter++;
			if (iter == num_1.end())
			{
				result.insert(result.begin(), num_2.size() - 1, '0');
				i = result.size() - 100;
				result.insert(i, 1, '.');
				while (*result.begin() == '0' && *(result.begin() + 1) != '.')
					result.erase(result.begin());
				while (*(result.end() - 1) == '0' && *(result.end() - 2) != '.') result.erase(result.end() - 1);
				break;
			}
			num1_tmp.assign(temp2);
			num1_tmp.insert(num1_tmp.end(), *iter);
			goto again;
		}
	}
	return result;
}
string Operation::factorial(string &num)
{
	string result(num);
	string step(1, '1');
	string _num(num);
	//  if(num.size()<=1)
	//   if(num[0]=='1' || num[0]=='0')
	//    return step;
	//  cout <<num<<endl;
	//  result.assign(multiplication(factorial(subduction(num,step)),num));
	for (_num.assign(subduction(_num, step)); _num.size() == 1 ? (_num[0] == '1' || _num[0] == '0' ? 0 : 1) : 1; _num.assign(subduction(_num, step)))
		result.assign(multiplication(result, _num));
	return result;
}
char Operation::Check_Oper(string &str, string &num_1, string &num_2)
{
	char ch = NULL;
	short k = 1;
	string::size_type i;
	if (str[str.size() - 1] == '!')
	if (str[0] >= 48 && str[0] <= 57)
		return '!';
	else
		return NULL;
	if (str[0] != '-') i = 0;
	else if (str[0] == '-')
	{
		num_1.insert(num_1.end(), '-');
		i = 1;
	}
	else if (str[0] == '+') i = 1;
	else return NULL;
	for (; i<str.size(); i++)
	{
		if (str[i] >= 48 && str[i] <= 57)
		{
			if (k) num_1.insert(num_1.end(), str[i]);
			else
				num_2.insert(num_2.end(), str[i]);
		}
		else
		{
			if (ch != NULL) return NULL;
			switch (str[i])
			{
			case '+':
				ch = '+';
				break;
			case '-':
				ch = '-';
				break;
			case '*':
				ch = '*';
				break;
			case '/':
				ch = '/';
				break;
			default:
				return NULL;
			}
			k = 0;
		}
	}
	return ch;
}
string Operation::Reverse_String(string &str)
{
	string::size_type i, j = str.size() - 1, length = str.size() / 2;
	char tmp = NULL;
	for (i = 0; i<length; i++, j--)
	{
		tmp = str[i];
		str[i] = str[j];
		str[j] = tmp;
	}
	return str;
}
string Operation::SUB_ASC(string &str)
{
	string::size_type i;
	for (i = 0; i<str.size(); i++) str[i] -= 48;
	return str;
}
string Operation::ADD_ASC(string &str)
{
	string::size_type i;
	for (i = 0; i<str.size(); i++) str[i] += 48;
	return str;
}
string Operation::GET_MAX(string &num1, string &num2)
{
	string num_1(num1);
	string num_2(num2);
	string::size_type i;
	while (num_1[0] == '0' || num_2[0] == '0')
	{
		if (num_1[0] == '0') num_1.erase(num_1.begin());
		if (num_2[0] == '0') num_2.erase(num_2.begin());
	}
	if (num_1.size()>num_2.size()) return num_1;
	else if (num_1.size()<num_2.size()) return num_2;
	else
	{
		for (i = 0; i != num_1.size(); i++)
		if (num_1[i]>num_2[i]) return num_1;
		else if (num_1[i]<num_2[i]) return num_2;
	}
	return num_1;
}
int Console()
{
	system("cls");
	SetConsoleTitle((LPCWSTR)"有符号大整型数字计算器  ——By:计科12-1班 魏子楠");
	cout << "===============================================================================" << endl;
	cout << "\t\t\t有符号大整型数字计算器" << endl;
	cout << "本程序可进行:\n1.加法运算\t2.减法运算\t3.乘法运算\t4.除法运算\t5.阶乘运算" << endl;
	cout << "===============================================================================" << endl;
	return 1;
}
int main(int argc, char *argv[])
{
	char ch;
	Operation oper;
again:
	string *_str = new string;
	string *_num1 = new string;
	string *_num2 = new string;
	string *_result = new string;
	string &str = *_str;
	string &num1 = *_num1;
	string &num2 = *_num2;
	string &result = *_result;
	Console();
	fflush(stdin);
	cout << "请输入运算式:" << endl;
	cin >> str;
	if ((ch = oper.Check_Oper(str, num1, num2)) == NULL) cout << "输入有误!" << endl;
	if (num1[0] == '-')
		switch (ch)
	{
		case '+':
			num1.erase(num1.begin());
			result.assign(oper.subduction(num2, num1));
			break;
		case '-':
			num1.erase(num1.begin());
			result.assign(oper.addition(num1, num2));
			result.insert(result.begin(), '-');
			break;
		case '*':
			num1.erase(num1.begin());
			result.assign(oper.multiplication(num1, num2));
			result.insert(result.begin(), '-');
			break;
		case '/':
			num1.erase(num1.begin());
			result.assign(oper.division(num1, num2));
			result.insert(result.begin(), '-');
			break;
		default:
			cout << "数据出错!" << endl;
			return -1;
	}
	else
		switch (ch)
	{
		case '+':
			result.assign(oper.addition(num1, num2));
			break;
		case '-':
			result.assign(oper.subduction(num1, num2));
			break;
		case '*':
			result.assign(oper.multiplication(num1, num2));
			break;
		case '/':
			result.assign(oper.division(num1, num2));
			break;
		case '!':
			str.erase(str.end() - 1);
			result.assign(oper.factorial(str));
			break;
		default:
			cout << "数据出错!" << endl;
			return -1;
	}
	cout << "计算结果:" << endl << result << endl;
	delete _str;
	delete _result;
	delete _num2;
	delete _num1;
	getch();
	goto again;
	return 1;
}
