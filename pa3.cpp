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

struct args
{
  int *parIdx;
  int index;
  pthread_mutex_t *m;
  pthread_cond_t *cond;
  char symbol;
  std::string code_integer;
  std::string code_binary;
  std::string encoded_message; 
  int *bit_length;             
  std::string *output_ptr;
};

struct args2
{
  int *parIdx;
  int index;
  pthread_mutex_t *m;
  pthread_cond_t *cond;
  std::map<std::string, char>* key;
  std::string encode_message;
  std::string* answer_ptr;
};

std::string help_convert(std::string code_integer, int *bit) // determine binary representation
{
  int decimal = stoi(code_integer);
  std::string binary;
  while (decimal > 0) 
  {
    binary.append(std::to_string(decimal % 2));
    decimal /= 2;
  }
  reverse(binary.begin(), binary.end()); // flip the string to display correct binary
  while (binary.length() < *bit)
  { // formating to match the fit-size bit length
    binary.insert(0, "0");
  }
  return binary;
}

int get_freq(std::string encoded_message, int *bit, std::string code_binary) //determine frequency
{
  int freq = 0;
  for (int i = 0; i < (encoded_message.length() - 2); i = i + *bit)
  {
    if (encoded_message.substr(i, *bit) == code_binary)
    { // take string begin at i, and bit number following i
      freq++;
    }
  }
  return freq;
}

void *work(void *arguments)
{
  args *child = (args *)arguments;
  int index = child->index;
  std::string code_integer = child->code_integer; 
  std::string code_binary;
  int frequency;
  char symbol = child->symbol;
  std::string *output_ptr = child->output_ptr;

  pthread_mutex_unlock(child->m);

  code_binary = help_convert(code_integer, child->bit_length);                  // convert decimal to binary
  frequency = get_freq(child->encoded_message, child->bit_length, code_binary); // get the frequency of binary segment
  *output_ptr = code_binary; //update array value for mapping later

  // FIRST CRITICAL SECTION
  pthread_mutex_lock(child->m);
  while (*child->parIdx != index)
  {
    pthread_cond_wait(child->cond, child->m);
  }
  pthread_mutex_unlock(child->m);

  std::cout << "Character: " << symbol << ", Code: " << code_binary << ", Frequency: " << frequency << std::endl;

  // SECOND CRITICAL SECTION
  pthread_mutex_lock(child->m);
  *(child->parIdx) = *(child->parIdx) + 1;  
  pthread_cond_broadcast(child->cond);
  pthread_mutex_unlock(child->m);
  return NULL;
}

void *decompress(void *arguments)
{
  args2 *child = (args2 *)arguments;
  int index = child->index;
  std::string encoded_message = child->encode_message;
  std::string *ans_ptr = child->answer_ptr;

  pthread_mutex_unlock(child->m);

  *ans_ptr= (*child->key)[encoded_message];

  // FIRST CRITICAL SECTION
  pthread_mutex_lock(child->m);
  while (*child->parIdx != index)
  {
    pthread_cond_wait(child->cond, child->m);
  }
  pthread_mutex_unlock(child->m);

  // SECOND CRITICAL SECTION
  pthread_mutex_lock(child->m);
  *(child->parIdx) = *(child->parIdx) + 1;
  pthread_cond_broadcast(child->cond);
  pthread_mutex_unlock(child->m);

  return NULL;
}

int main()
{
  // initializing
  int n;
  std::string line;
  int greatest_base_10 = 0;
  std::string encoded_message;
  int bit_length;

  std::cin >> n;
  std::cin.ignore();

  char symbol[n];
  std::string integer[n];

  for (int i = 0; i < n; i++)
  { // getting symbols from output
    getline(std::cin, line);
    symbol[i] = line[0];
    integer[i] = line.substr(2);

    if (greatest_base_10 < stoi(integer[i]))
    {
      greatest_base_10 = stoi(integer[i]);
    }
  }
  bit_length = ceil(log2(greatest_base_10 + 1)); // get fixed bit formula
  getline(std::cin, line);
  encoded_message = line;

  static pthread_mutex_t mutex;
  static pthread_cond_t turn;
  struct args threadArgs;
  threadArgs.m = &mutex;
  threadArgs.cond = &turn;
  int parIdx = 0;
  threadArgs.encoded_message = encoded_message;
  threadArgs.bit_length = &bit_length;
  pthread_t threads[n];

  std::string binary_output[n];

  std::map<std::string, char> key;
  std::cout << "Alphabet:" << std::endl;
  for (int i = 0; i < n; i++)
  {
    pthread_mutex_lock(&mutex);
    threadArgs.parIdx = &parIdx;
    threadArgs.index = i;
    threadArgs.symbol = symbol[i];
    threadArgs.code_integer = integer[i];
    threadArgs.output_ptr = &binary_output[i];
    pthread_create(&threads[i], NULL, work, &threadArgs);
  }

  for (int i = 0; i < n; i++)
  {
    pthread_join(threads[i], NULL);
  }

  for (int i = 0; i < n; i++) // populate map
  {
    key[binary_output[i]] = symbol[i];
  }

  struct args2 threadArgs2;
  threadArgs2.m = &mutex;
  threadArgs2.cond = &turn;
  int num_of_encoded = encoded_message.size() / bit_length;
  pthread_t threads2[num_of_encoded];
  std::string compress_message[num_of_encoded];
  // Assigning tasks (binary sequence to be decode in thread)
  int j = 0;
  for (int i = 0; i < encoded_message.length(); i += bit_length)
  {
    compress_message[j] = encoded_message.substr(i, bit_length);
    // std::cout << compress_message[j] << std::endl;
    j++;
  }

  parIdx = 0;
  threadArgs2.key = &key;
  std::cout << std::endl;
  std::cout << "Decompressed message: ";
  std::string answer[num_of_encoded];

  for (int i = 0; i < num_of_encoded; i++)
  {
    pthread_mutex_lock(&mutex);
    threadArgs2.parIdx = &parIdx;
    threadArgs2.index = i; // assign index
    threadArgs2.encode_message = compress_message[i];
    threadArgs2.answer_ptr = &answer[i]; //pass pointer to struct
    pthread_create(&threads2[i], NULL, decompress, &threadArgs2);
  }
  for (int i = 0; i < num_of_encoded; i++)
    pthread_join(threads2[i], NULL);
    
  for(int i = 0; i < num_of_encoded; i++)
    std::cout << answer[i];
  return 0;
}
