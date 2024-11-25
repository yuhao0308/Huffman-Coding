#include <iostream>
#include <cstdio>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <omp.h>
#include <vector>
#include "progress_bar.hpp"

using namespace std;

void write_from_uChar(unsigned char, unsigned char &, int &, FILE *);
void write_from_uChar(unsigned char, unsigned char &, int &, vector<unsigned char> &);

int this_is_not_a_folder(char *);
long int size_of_the_file(char *);
void count_in_folder(string, long int *, long int &, long int &);

void write_file_count(int, unsigned char &, int &, FILE *);
void write_file_size(long int, unsigned char &, int &, vector<unsigned char> &);
void write_file_name(char *, string *, unsigned char &, int &, vector<unsigned char> &);
void write_the_file_content(FILE *, long int, string *, unsigned char &, int &, vector<unsigned char> &);
void write_the_folder(string, string *, unsigned char &, int &, vector<unsigned char> &);

progress PROGRESS;

struct ersel
{ // this structure will be used to create the translation tree
  ersel *left, *right;
  long int number;
  unsigned char character;
  string bit;
};

bool erselcompare0(ersel a, ersel b)
{
  return a.number < b.number;
}

// Function to assign codes to the Huffman tree
void assign_codes(ersel *node, const string &code)
{
  if (!node)
    return;
  node->bit = code;
  assign_codes(node->left, code + "1");
  assign_codes(node->right, code + "0");
}

int main(int argc, char *argv[])
{
  long int number[256] = {0};
  long int total_bits = 0;
  int letter_count = 0;
  if (argc == 1)
  {
    cout << "Missing file name" << endl
         << "try './archive {{file_name}}'" << endl;
    return 0;
  }

  string scompressed;
  FILE *original_fp, *compressed_fp;

  // Check for wrong input
  for (int i = 1; i < argc; i++)
  {
    if (this_is_not_a_folder(argv[i]))
    {
      original_fp = fopen(argv[i], "rb");
      if (!original_fp)
      {
        cout << argv[i] << " file does not exist" << endl
             << "Process has been terminated" << endl;
        return 0;
      }
      fclose(original_fp);
    }
  }

  scompressed = argv[1];
  scompressed += ".compressed";

  unsigned char x;
  long int total_size = 0, size;
  total_bits += 16 + 9 * (argc - 1);

  // Parallel region for counting byte frequencies
  long int total_number[256] = {0};
  long int global_total_size = 0;
  long int global_total_bits = 0;

#pragma omp parallel
  {
    long int local_number[256] = {0};
    long int local_total_size = 0;
    long int local_total_bits = 0;

#pragma omp for schedule(dynamic) nowait
    for (int current_file = 1; current_file < argc; current_file++)
    {

      for (char *c = argv[current_file]; *c; c++)
      { // counting usage frequency of unique bytes on the file name (or folder name)
        local_number[(unsigned char)(*c)]++;
      }

      if (this_is_not_a_folder(argv[current_file]))
      {
        local_total_size += size = size_of_the_file(argv[current_file]);
        local_total_bits += 64;

        FILE *original_fp = fopen(argv[current_file], "rb");
        unsigned char x;
        for (long int j = 0; j < size; j++)
        {
          fread(&x, 1, 1, original_fp);
          local_number[x]++;
        }
        fclose(original_fp);
      }
      else
      {
        string temp = argv[current_file];
        count_in_folder(temp, local_number, local_total_size, local_total_bits);
      }
    }

#pragma omp critical
    {
      for (int i = 0; i < 256; i++)
      {
        total_number[i] += local_number[i];
      }
      global_total_size += local_total_size;
      global_total_bits += local_total_bits;
    }
  }

  memcpy(number, total_number, sizeof(number));
  total_size += global_total_size;
  total_bits += global_total_bits;

  for (long int *i = number; i < number + 256; i++)
  {
    if (*i)
    {
      letter_count++;
    }
  }

  // Creating the base of the translation array
  ersel array[512]; // Maximum size considering worst case
  int array_size = 0;
  ersel *e = array;
  for (long int *i = number; i < number + 256; i++)
  {
    if (*i)
    {
      e->right = NULL;
      e->left = NULL;
      e->number = *i;
      e->character = i - number;
      e++;
      array_size++;
    }
  }
  sort(array, array + array_size, erselcompare0);

  // Building the Huffman tree
  ersel *min1 = array, *min2 = array + 1, *current = array + array_size, *notleaf = array + array_size, *isleaf = array + 2;
  int total_nodes = array_size;
  for (int i = 0; i < array_size - 1; i++)
  {
    current->number = min1->number + min2->number;
    current->left = min1;
    current->right = min2;
    min1->bit = "1";
    min2->bit = "0";
    current++;
    total_nodes++;

    if (isleaf >= array + array_size)
    {
      min1 = notleaf;
      notleaf++;
    }
    else
    {
      if (isleaf->number < notleaf->number)
      {
        min1 = isleaf;
        isleaf++;
      }
      else
      {
        min1 = notleaf;
        notleaf++;
      }
    }

    if (isleaf >= array + array_size)
    {
      min2 = notleaf;
      notleaf++;
    }
    else if (notleaf >= current)
    {
      min2 = isleaf;
      isleaf++;
    }
    else
    {
      if (isleaf->number < notleaf->number)
      {
        min2 = isleaf;
        isleaf++;
      }
      else
      {
        min2 = notleaf;
        notleaf++;
      }
    }
  }

  // Code assignment
  ersel *root = current - 1;
  assign_codes(root, "");

  compressed_fp = fopen(&scompressed[0], "wb");
  int current_bit_count = 0;
  unsigned char current_byte = 0;

  // Writing first
  fwrite(&letter_count, 1, 1, compressed_fp);
  total_bits += 8;

  // Writing second (password handling remains unchanged)
  {
    cout << "If you want a password write any number other than 0" << endl
         << "If you do not, write 0" << endl;
    int check_password;
    cin >> check_password;
    if (check_password)
    {
      string password;
      cout << "Enter your password (Do not use whitespaces): ";
      cin >> password;
      int password_length = password.length();
      if (password_length == 0)
      {
        cout << "You did not enter a password" << endl
             << "Process has been terminated" << endl;
        fclose(compressed_fp);
        remove(&scompressed[0]);
        return 0;
      }
      if (password_length > 100)
      {
        cout << "Password cannot contain more than 100 characters" << endl
             << "Process has been terminated" << endl;
        fclose(compressed_fp);
        remove(&scompressed[0]);
        return 0;
      }
      unsigned char password_length_unsigned = password_length;
      fwrite(&password_length_unsigned, 1, 1, compressed_fp);
      fwrite(&password[0], 1, password_length, compressed_fp);
      total_bits += 8 + 8 * password_length;
    }
    else
    {
      fwrite(&check_password, 1, 1, compressed_fp);
      total_bits += 8;
    }
  }

  // Writing third (translation script)
  char *str_pointer;
  unsigned char len, current_character;
  string str_arr[256];
  for (e = array; e < array + array_size; e++)
  {
    str_arr[(e->character)] = e->bit; // Storing the transformation string
    len = e->bit.length();
    current_character = e->character;

    write_from_uChar(current_character, current_byte, current_bit_count, compressed_fp);
    write_from_uChar(len, current_byte, current_bit_count, compressed_fp);
    total_bits += len + 16;

    str_pointer = &e->bit[0];
    while (*str_pointer)
    {
      if (current_bit_count == 8)
      {
        fwrite(&current_byte, 1, 1, compressed_fp);
        current_byte = 0;
        current_bit_count = 0;
      }
      switch (*str_pointer)
      {
      case '1':
        current_byte <<= 1;
        current_byte |= 1;
        current_bit_count++;
        break;
      case '0':
        current_byte <<= 1;
        current_bit_count++;
        break;
      default:
        cout << "An error has occurred" << endl
             << "Compression process aborted" << endl;
        fclose(compressed_fp);
        remove(&scompressed[0]);
        return 1;
      }
      str_pointer++;
    }
  }
  if (total_bits % 8)
  {
    total_bits = (total_bits / 8 + 1) * 8;
  }

  cout << "The size of the sum of ORIGINAL files is: " << total_size << " bytes" << endl;
  cout << "The size of the COMPRESSED file will be: " << total_bits / 8 << " bytes" << endl;
  cout << "Compressed file's size will be [%" << 100 * ((float)total_bits / 8 / total_size) << "] of the original file" << endl;
  if (total_bits / 8 > total_size)
  {
    cout << endl
         << "COMPRESSED FILE'S SIZE WILL BE HIGHER THAN THE SUM OF ORIGINALS" << endl
         << endl;
  }
  cout << "If you wish to abort this process write 0 and press enter" << endl
       << "If you want to continue write any other number and press enter" << endl;
  int check;
  cin >> check;
  if (!check)
  {
    cout << endl
         << "Process has been aborted" << endl;
    fclose(compressed_fp);
    remove(&scompressed[0]);
    return 0;
  }

  PROGRESS.MAX = root->number; // setting progress bar

  // Writing fourth
  write_file_count(argc - 1, current_byte, current_bit_count, compressed_fp);

  // Prepare thread-local buffers
  int num_files = argc - 1;
  vector<vector<unsigned char>> thread_buffers(num_files);
  vector<int> buffer_bit_counts(num_files, 0);

  // Parallel file compression
#pragma omp parallel for schedule(dynamic)
  for (int current_file = 1; current_file < argc; current_file++)
  {
    vector<unsigned char> &local_buffer = thread_buffers[current_file - 1];
    int &local_bit_count = buffer_bit_counts[current_file - 1];
    unsigned char local_current_byte = 0;

    if (this_is_not_a_folder(argv[current_file]))
    { // if current is a file and not a folder
      FILE *local_original_fp = fopen(argv[current_file], "rb");
      fseek(local_original_fp, 0, SEEK_END);
      long int size = ftell(local_original_fp);
      rewind(local_original_fp);

      // Writing fifth
      local_current_byte <<= 1;
      local_current_byte |= 1;
      local_bit_count++;

      // Write file size
      for (int i = 0; i < 8; i++)
      {
        unsigned char temp = (size >> ((7 - i) * 8)) & 0xFF;
        write_from_uChar(temp, local_current_byte, local_bit_count, local_buffer);
      }

      // Write file name
      write_file_name(argv[current_file], str_arr, local_current_byte, local_bit_count, local_buffer);

      // Write file content
      write_the_file_content(local_original_fp, size, str_arr, local_current_byte, local_bit_count, local_buffer);

      fclose(local_original_fp);
    }
    else
    { // if current is a folder
      // Writing fifth
      local_current_byte <<= 1;
      local_bit_count++;

      // Write folder name
      write_file_name(argv[current_file], str_arr, local_current_byte, local_bit_count, local_buffer);

      // Write folder content
      write_the_folder(argv[current_file], str_arr, local_current_byte, local_bit_count, local_buffer);
    }

    // Flush remaining bits in local buffer
    if (local_bit_count > 0 && local_bit_count < 8)
    {
      local_current_byte <<= (8 - local_bit_count);
      local_buffer.push_back(local_current_byte);
      local_bit_count = 0;
    }
  }

  // Write buffers to compressed file sequentially
  for (int i = 0; i < num_files; i++)
  {
    vector<unsigned char> &local_buffer = thread_buffers[i];
    fwrite(&local_buffer[0], 1, local_buffer.size(), compressed_fp);
  }

  // Flush remaining bits
  if (current_bit_count > 0 && current_bit_count < 8)
  {
    current_byte <<= (8 - current_bit_count);
    fwrite(&current_byte, 1, 1, compressed_fp);
  }

  fclose(compressed_fp);
  system("clear");
  cout << endl
       << "Created compressed file: " << scompressed << endl;
  cout << "Compression is complete" << endl;

  return 0;
}

// Modified functions to support thread-local buffers

void write_from_uChar(unsigned char uChar, unsigned char &current_byte, int &current_bit_count, vector<unsigned char> &buffer)
{
  for (int i = 0; i < 8; i++)
  {
    if (current_bit_count == 8)
    {
      buffer.push_back(current_byte);
      current_byte = 0;
      current_bit_count = 0;
    }
    current_byte <<= 1;
    current_byte |= (uChar >> (7 - i)) & 1;
    current_bit_count++;
  }
}

void write_from_uChar(unsigned char uChar, unsigned char &current_byte, int &current_bit_count, FILE *fp_write)
{
  for (int i = 0; i < 8; i++)
  {
    if (current_bit_count == 8)
    {
      fwrite(&current_byte, 1, 1, fp_write);
      current_byte = 0;
      current_bit_count = 0;
    }
    current_byte <<= 1;
    current_byte |= (uChar >> (7 - i)) & 1;
    current_bit_count++;
  }
}

void write_file_count(int file_count, unsigned char &current_byte, int &current_bit_count, FILE *compressed_fp)
{
  unsigned char temp = file_count % 256;
  write_from_uChar(temp, current_byte, current_bit_count, compressed_fp);
  temp = file_count / 256;
  write_from_uChar(temp, current_byte, current_bit_count, compressed_fp);
}

void write_file_size(long int size, unsigned char &current_byte, int &current_bit_count, vector<unsigned char> &buffer)
{
  for (int i = 0; i < 8; i++)
  {
    unsigned char temp = (size >> ((7 - i) * 8)) & 0xFF;
    write_from_uChar(temp, current_byte, current_bit_count, buffer);
  }
}

void write_file_name(char *file_name, string *str_arr, unsigned char &current_byte, int &current_bit_count, vector<unsigned char> &buffer)
{
  write_from_uChar(strlen(file_name), current_byte, current_bit_count, buffer);
  char *str_pointer;
  for (char *c = file_name; *c; c++)
  {
    str_pointer = &str_arr[(unsigned char)(*c)][0];
    while (*str_pointer)
    {
      if (current_bit_count == 8)
      {
        buffer.push_back(current_byte);
        current_byte = 0;
        current_bit_count = 0;
      }
      switch (*str_pointer)
      {
      case '1':
        current_byte <<= 1;
        current_byte |= 1;
        current_bit_count++;
        break;
      case '0':
        current_byte <<= 1;
        current_bit_count++;
        break;
      default:
        cout << "An error has occurred" << endl
             << "Process has been aborted";
        exit(2);
      }
      str_pointer++;
    }
  }
}

void write_the_file_content(FILE *original_fp, long int size, string *str_arr, unsigned char &current_byte, int &current_bit_count, vector<unsigned char> &buffer)
{
  unsigned char x;
  char *str_pointer;
  for (long int i = 0; i < size; i++)
  {
    fread(&x, 1, 1, original_fp);
    str_pointer = &str_arr[x][0];
    while (*str_pointer)
    {
      if (current_bit_count == 8)
      {
        buffer.push_back(current_byte);
        current_byte = 0;
        current_bit_count = 0;
      }
      switch (*str_pointer)
      {
      case '1':
        current_byte <<= 1;
        current_byte |= 1;
        current_bit_count++;
        break;
      case '0':
        current_byte <<= 1;
        current_bit_count++;
        break;
      default:
        cout << "An error has occurred" << endl
             << "Process has been aborted";
        exit(2);
      }
      str_pointer++;
    }
  }
}

void write_the_folder(string path, string *str_arr, unsigned char &current_byte, int &current_bit_count, vector<unsigned char> &buffer)
{
  FILE *original_fp;
  path += '/';
  DIR *dir = opendir(&path[0]), *next_dir;
  string next_path;
  struct dirent *current;
  int file_count = 0;
  long int size;
  while ((current = readdir(dir)))
  {
    if (current->d_name[0] == '.')
    {
      if (current->d_name[1] == 0)
        continue;
      if (current->d_name[1] == '.' && current->d_name[2] == 0)
        continue;
    }
    file_count++;
  }
  rewinddir(dir);
  // Writes fourth
  unsigned char temp = file_count % 256;
  write_from_uChar(temp, current_byte, current_bit_count, buffer);
  temp = file_count / 256;
  write_from_uChar(temp, current_byte, current_bit_count, buffer);

  while ((current = readdir(dir)))
  { // if current is a file
    if (current->d_name[0] == '.')
    {
      if (current->d_name[1] == 0)
        continue;
      if (current->d_name[1] == '.' && current->d_name[2] == 0)
        continue;
    }

    next_path = path + current->d_name;
    if (this_is_not_a_folder(&next_path[0]))
    {
      original_fp = fopen(&next_path[0], "rb");
      fseek(original_fp, 0, SEEK_END);
      size = ftell(original_fp);
      rewind(original_fp);

      // Writing fifth
      current_byte <<= 1;
      current_byte |= 1;
      current_bit_count++;

      write_file_size(size, current_byte, current_bit_count, buffer);                              // writes sixth
      write_file_name(current->d_name, str_arr, current_byte, current_bit_count, buffer);          // writes seventh
      write_the_file_content(original_fp, size, str_arr, current_byte, current_bit_count, buffer); // writes eighth
      fclose(original_fp);
    }
    else
    { // if current is a folder
      // Writing fifth
      current_byte <<= 1;
      current_bit_count++;

      write_file_name(current->d_name, str_arr, current_byte, current_bit_count, buffer); // writes seventh

      write_the_folder(next_path, str_arr, current_byte, current_bit_count, buffer);
    }
  }
  closedir(dir);
}

int this_is_not_a_folder(char *path)
{
  DIR *temp = opendir(path);
  if (temp)
  {
    closedir(temp);
    return 0;
  }
  return 1;
}

long int size_of_the_file(char *path)
{
  long int size;
  FILE *fp = fopen(path, "rb");
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fclose(fp);
  return size;
}

void count_in_folder(string path, long int *local_number, long int &local_total_size, long int &local_total_bits)
{
  FILE *original_fp;
  path += '/';
  DIR *dir = opendir(&path[0]), *next_dir;
  string next_path;
  local_total_size += 4096;
  local_total_bits += 16; // for file_count
  struct dirent *current;
  while ((current = readdir(dir)))
  {
    if (current->d_name[0] == '.')
    {
      if (current->d_name[1] == 0)
        continue;
      if (current->d_name[1] == '.' && current->d_name[2] == 0)
        continue;
    }
    local_total_bits += 9;

    for (char *c = current->d_name; *c; c++)
    { // counting usage frequency of bytes on the file name (or folder name)
      local_number[(unsigned char)(*c)]++;
    }

    next_path = path + current->d_name;

    if ((next_dir = opendir(&next_path[0])))
    {
      closedir(next_dir);
      count_in_folder(next_path, local_number, local_total_size, local_total_bits);
    }
    else
    {
      long int size;
      unsigned char x;
      local_total_size += size = size_of_the_file(&next_path[0]);
      local_total_bits += 64;

      original_fp = fopen(&next_path[0], "rb");

      for (long int j = 0; j < size; j++)
      { // counting usage frequency of bytes inside the file
        fread(&x, 1, 1, original_fp);
        local_number[x]++;
      }
      fclose(original_fp);
    }
  }
  closedir(dir);
}