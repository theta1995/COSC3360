#include <iostream>
#include <pthread.h>
#include <string>
#include <cmath>
#include <unistd.h>
#include <algorithm>
#include <list>
#include <cstring>
#include <array>
#include <map>

struct arguments
{
  char symbol;  //alphabet
  std::string code_integer; //in as decimal, out as binary
  std::string binary_message; //given binary message
  int bit_length; //fixed size
  int frequency;  //freq counter
};

struct dictionary
{
  std::map<std::string, std::string> key; //map {binary_sequence, alphabet}
  std::string encoded_message; // binary in
  std::string decoded_message; // alphabet out
};

// Thread set #1
void *toBinary_countFrequency(void *arg)
{
  // initizalizing
  struct arguments *encrypted_text = (arguments *)arg;
  int decimal = stoi(encrypted_text->code_integer);
  int bit = encrypted_text->bit_length;
  int freq;
  std::string localMessage = encrypted_text->binary_message;
  std::string binary;

  while (decimal > 0) // convert decimal to binary
  {
    binary.append(std::to_string(decimal % 2));
    decimal /= 2;
  }
  reverse(binary.begin(), binary.end()); // flip the string to display correct binary
  while (binary.length() < bit)
  { // formating to match the fit-size bit length
    binary.insert(0, "0");
  }
  encrypted_text->code_integer = binary;

  for (int i = 0; i < (localMessage.length() - 2); i = i + bit)
  {
    if (localMessage.substr(i, bit) == binary)
    { // take string begin at i, and bit number following i
      freq++;
    }
  }
  encrypted_text->frequency = freq;

  return 0;
}

void *decompress(void *arg)
{

  struct dictionary *message = (dictionary *)arg;
  std::map<std::string, std::string> key = message->key;
  message->decoded_message = key[message->encoded_message];

  return 0;
}

int main()
{

  // initializing
  int n;
  std::string line;
  int greatest_base_10 = 0;
  std::string message;
  int bit_length;

  std::cin >> n;
  std::cin.ignore();

  struct arguments input[n];

  for (int i = 0; i < n; i++)
  { // getting symbols from output
    getline(std::cin, line);
    input[i].symbol = line[0];
    input[i].code_integer = line.substr(2);

    if (greatest_base_10 < stoi(input[i].code_integer))
    {
      greatest_base_10 = stoi(input[i].code_integer);
    }
  }
  getline(std::cin, line);
  message = line;

  // insert binary message and fix-length bit into struct
  for (int i = 0; i < n; i++)
  {
    input[i].binary_message = message;
    bit_length = ceil(log2(greatest_base_10 + 1));
    input[i].bit_length = bit_length;
  }

  // threads for finding binary
  pthread_t tid1[n];
  for (int i = 0; i < n; i++)
  {
    if (pthread_create(&tid1[i], nullptr, toBinary_countFrequency, &input[i]) != 0)
    {
      perror("Problem encountered while creating first set of thread");
    }
  }
  for (int i = 0; i < n; i++)
  {
    if (pthread_join(tid1[i], nullptr) != 0)
    {
      perror("Problem encountered while joining first set of thread");
    }
  }

  //Initalizing for preparation of 2nd thread
  int num_of_encoded = message.size() / bit_length;
  struct dictionary compress_message[num_of_encoded];
  pthread_t tid2[num_of_encoded];

  //Assigning tasks (binary sequence to be decode in thread)
  int j = 0;
  for (int i = 0; i < message.length(); i += bit_length)
  {
    compress_message[j].encoded_message = message.substr(i, bit_length);
    j++;
  }
  //Initialize and populate map with alphabet and it's coresponding binary value
  std::map<std::string, std::string> key;
  for (int i = 0; i < n; i++)
  {
    key[input[i].code_integer] = input[i].symbol;
  }
  for (int i = 0; i < num_of_encoded; i++)
  {
    compress_message[i].key = key;
  }
  //thread for decompressing
  for (int i = 0; i < num_of_encoded; i++)
  {
    if (pthread_create(&tid2[i], nullptr, decompress, &compress_message[i]) != 0)
    {
      perror("Problem encountered while creating first set of thread");
    }
  }
  for (int i = 0; i < num_of_encoded; i++)
  {
    if (pthread_join(tid2[i], nullptr) != 0)
    {
      perror("Problem encountered while joining first set of thread");
    }
  }

  //-------Begin Formating -------------
  std::cout << "Alphabet:" << std::endl;
  for (int i = 0; i < n; i++)
  {
    std::cout << "Character: " << input[i].symbol << ", Code: " << input[i].code_integer << ", Frequency: " << input[i].frequency << std::endl;
  }
  std::string full_deciphered_msg;
  for (int i = 0; i < num_of_encoded; i++)
  {
    full_deciphered_msg += compress_message[i].decoded_message; // concatenate all alphabet
  }
  std::cout << std::endl;
  std::cout << "Decompressed message: " << full_deciphered_msg << std::endl;
  ;
  //---------End Formating--------------

  return 0;
}
