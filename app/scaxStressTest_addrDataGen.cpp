#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <sstream>

std::string convert_addr(unsigned int n) // padding the address
{
  std::stringstream ss;
  std::stringstream ssFinal;
  std::string stringTmp;

  ss << std::hex << std::uppercase << n;

  stringTmp = ss.str();

  if(stringTmp.size() == 1)
  {
    ssFinal << "0" << "0" << "0" << stringTmp;
  }
  else if(stringTmp.size() == 2)
  {
    ssFinal << "0" << "0" << stringTmp;
  }
  else if(stringTmp.size() == 3)
  {
    ssFinal << "0" << stringTmp;
  }

  return ssFinal.str();
}

std::string convert_data(unsigned int n) // padding the data
{
  std::stringstream ss;
  std::stringstream ssFinal;
  std::string stringTmp;

  ss << std::hex << std::uppercase << n;

  stringTmp = ss.str();

  if(stringTmp.size() == 1)
  {
    ssFinal << "0" << "0" << "0" << "0" << "0" << "0" << "0" << stringTmp;
  }
  else if(stringTmp.size() == 2)
  {
    ssFinal << "0" << "0" << "0" << "0" << "0" << "0" << stringTmp;
  }
  else if(stringTmp.size() == 3)
  {
    ssFinal << "0" << "0" << "0" << "0" << "0" << stringTmp;
  }
  else if(stringTmp.size() == 4)
  {
    ssFinal << "0" << "0" << "0" << "0" << stringTmp;
  }
  else if(stringTmp.size() == 5)
  {
    ssFinal << "0" << "0" << "0" << stringTmp;
  }
  else if(stringTmp.size() == 6)
  {
    ssFinal << "0" << "0" << stringTmp;
  }
  else if(stringTmp.size() == 7)
  {
    ssFinal << "0" << stringTmp;
  }
  else if(stringTmp.size() == 8)
  {
    ssFinal << stringTmp;
  }

  return ssFinal.str();
}

int main()
{

  std::vector<std::string> addrVectorInitial;
  std::vector<std::string> addrVectorFinal;
  std::vector<std::string> dataVector;

  std::string stringConv;
  std::string stringToSave;

  srand (time(NULL));

  unsigned int randData = 0;
  unsigned int randAddr = 0;
  std::vector<unsigned int> usedIndices;

  for(unsigned int i = 0; i < 1024; i++)
  {

    addrVectorInitial.push_back(convert_addr(i));

    randData = rand() % 4294967295 + 33554431;

    dataVector.push_back(convert_data(randData));
  }

  while(addrVectorFinal.size() < 1024) // have we finished yet?
  {
    randAddr = rand() % 1024; // randomizer
    bool isUsed = false;

    if(usedIndices.size() > 0)
    {
      for(unsigned int j = 0; j < usedIndices.size(); j++)
      {
        if(randAddr == usedIndices[j]) // check if that random number we got corresponds to an address already pushed into the final address vector
        {
          isUsed = true;
          break;
        }
      }
    }

    if(!isUsed) // if we have not registered that address into the vector, push it back
    {
      addrVectorFinal.push_back(addrVectorInitial[randAddr]);
      usedIndices.push_back(randAddr);
    }
  }

  for(unsigned int i = 0; i < addrVectorFinal.size(); i++)
  {
    std::cout <<  addrVectorFinal[i] << ", " << dataVector[i] << std::endl; // the address-data pairs
  }

  // for(unsigned int i = 0; i < addrVectorFinal.size(); i++)
  // {
  //   std::cout << "r, " << master << ", " << addrVectorFinal[i] << std::endl; // the reads
  // }
}