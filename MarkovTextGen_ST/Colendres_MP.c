#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "murmur3.c"
#include <time.h>

//decide what to do with punctuation marks
typedef struct dict_entry{
  char word[32];
  int total_num_suffix;
  struct suff_entry *suffix[16];
  struct dict_entry *next;
} dict_entry;
typedef struct suff_entry{
  char word[32];
  int total_num_suffix;
  struct suff_entry *next;
} suff_entry;
typedef struct start_entry{
  char word[32];
  int count;
  struct start_entry *next;
} start_entry;


int main()
{
  FILE *f;

  char buf;
  char prefix[32];
  char suffix[32];
  int i = 0;
  int cnt = 0;
  int finalhash;
  uint32_t hash;
  uint32_t suffixhash;
  int finalsuffixhash;
  int totalStart = 0;
  uint32_t seed = 42;
  struct timespec startTime, endTime;
  int filenum;
  char filename[10] = "00000.txt";

  dict_entry dictionary[32768];
  start_entry startWord[32768];
  for (cnt = 0; cnt < 32768; cnt++)
  {
    dictionary[cnt].word[0] = '\0';
    dictionary[cnt].total_num_suffix = 0;
    dictionary[cnt].next = NULL;
    for(int sufcnt = 0; sufcnt < 16; sufcnt++)
    {
      dictionary[cnt].suffix[sufcnt] = NULL;
    }
    startWord[cnt].word[0] = '\0';
    startWord[cnt].count = 0;
    startWord[cnt].next = NULL;
  }
  //printf("made it past init.\n");
  dict_entry *temp;
  dict_entry *pretemp;
  suff_entry *spretemp;
  suff_entry *stemp;
  start_entry *startTemp;

  clock_gettime(CLOCK_REALTIME,&startTime);

  for(filenum = 0; filenum <= 395; filenum++)
  {
    //printf("%c\n", filename[4]);
    filename[0] = '0' + ((filenum%100000)/10000);
    filename[1] = '0' + ((filenum%10000)/1000);
    filename[2] = '0' + ((filenum%1000)/100);
    filename[3] = '0' + ((filenum%100)/10);
    filename[4] = '0' + filenum%10;

    f = fopen(filename, "rb");
    buf = fgetc(f);
    prefix[0] = '\0';
    suffix[0] = '\0';

    while(buf != EOF)
    {
      if((isspace(buf) == 0))
      {
        //printf("%d, %c\n",buf, buf);
        if(buf > 0)
        {
          suffix[i] = buf;
          i++;
        }

      }
      else
      {
        suffix[i] = '\0';
        //printf("Prefix: %s, Suffix: %s\n", prefix, suffix); //It's at this point you want to do dictionary operations
        MurmurHash3_x86_32(prefix, strlen(prefix), seed, &hash);
        finalhash = hash&(0x00007FFF);
        MurmurHash3_x86_32(suffix, strlen(suffix), seed, &suffixhash);
        finalsuffixhash = suffixhash&(0x0000000F);
        if(prefix[0] != '\0') //the "else" for this should be to populate the vector outlining the start characteristics/probabilities
        {
          if((*(dictionary[finalhash].word) == '\0'))
          {
            //printf("Populating empty bucket\n");
            strncpy(dictionary[finalhash].word, prefix, strlen(prefix));
            dictionary[finalhash].word[strlen(prefix)] = '\0';
            dictionary[finalhash].total_num_suffix = dictionary[finalhash].total_num_suffix + 1;
            dictionary[finalhash].next = NULL;
            dictionary[finalhash].suffix[finalsuffixhash] = (suff_entry*)malloc(sizeof(suff_entry));
            strncpy(dictionary[finalhash].suffix[finalsuffixhash]->word, suffix, strlen(suffix));
            dictionary[finalhash].suffix[finalsuffixhash]->word[strlen(suffix)] = '\0';
            dictionary[finalhash].suffix[finalsuffixhash]->total_num_suffix += 1;
            //dictionary[finalhash].suffix[finalsuffixhash]->suffix = NULL;
            dictionary[finalhash].suffix[finalsuffixhash]->next = NULL;
          }
          else if (!(strcmp(dictionary[finalhash].word,prefix)))
          {
            //printf("First bucket entry is the prefix\n");
            dictionary[finalhash].total_num_suffix = dictionary[finalhash].total_num_suffix + 1;
            if(dictionary[finalhash].suffix[finalsuffixhash] == NULL)
            {
              //printf("Populating empty suffix bucket\n");
              dictionary[finalhash].suffix[finalsuffixhash] = (suff_entry*)malloc(sizeof(suff_entry));
              strncpy(dictionary[finalhash].suffix[finalsuffixhash]->word, suffix, strlen(suffix));
              dictionary[finalhash].suffix[finalsuffixhash]->word[strlen(suffix)] = '\0';
              dictionary[finalhash].suffix[finalsuffixhash]->total_num_suffix += 1;
              //dictionary[finalhash].suffix[finalsuffixhash]->suffix = NULL;
              dictionary[finalhash].suffix[finalsuffixhash]->next = NULL;
            }
            else
            {
              if(!(strcmp(dictionary[finalhash].suffix[finalsuffixhash]->word, suffix)))
              {
                //printf("First suffix hash table entry is the suffix\n");
                dictionary[finalhash].suffix[finalsuffixhash]->total_num_suffix += 1;
              }
              else
              {
                //printf("Looking for suffix entry in suffix hash table entry linked list\n");
                stemp = dictionary[finalhash].suffix[finalsuffixhash];
                while(stemp->next != NULL)
                {
                  if(!(strcmp(stemp->word, suffix)))
                  {
                    break;
                  }
                  stemp = stemp->next;
                }
                if(!(strcmp(stemp->word, suffix)))
                {
                  //printf("Suffix exists in suffix linked list\n");
                  stemp->total_num_suffix += 1;
                }
                else
                {
                  //printf("Creating new suffix entry in linked list\n");
                  stemp->next = (suff_entry*)malloc(sizeof(suff_entry));
                  strncpy(stemp->next->word, suffix, strlen(suffix));
                  stemp->next->word[strlen(suffix)] = '\0';
                  stemp->next->total_num_suffix +=1;
                  //temp->next->suffix = NULL;
                  stemp->next->next = NULL;
                }
            }

            }
          }
          else
          {
            //printf("First prefix entry is not the prefix\n" );
            pretemp = &dictionary[finalhash];
            while(pretemp->next != NULL)
            {
              if(!(strcmp(pretemp->word, prefix)))
              {
                break;
              }
              pretemp = pretemp->next;

            }
            if(!(strcmp(pretemp->word, prefix)))
            {
              //printf("Prefix entry exists\n");
              pretemp->total_num_suffix++;
              if(pretemp->suffix[finalsuffixhash] == NULL)
              {
                //printf("Populating empty suffix bucket\n");
                dictionary[finalhash].suffix[finalsuffixhash] = (suff_entry*)malloc(sizeof(suff_entry));
                strncpy(dictionary[finalhash].suffix[finalsuffixhash]->word, suffix, strlen(suffix));
                dictionary[finalhash].suffix[finalsuffixhash]->word[strlen(suffix)] = '\0';
                dictionary[finalhash].suffix[finalsuffixhash]->total_num_suffix += 1;
                //dictionary[finalhash].suffix[finalsuffixhash]->suffix = NULL;
                dictionary[finalhash].suffix[finalsuffixhash]->next = NULL;

              }
              else
              {
                if(!(strcmp(pretemp->suffix[finalsuffixhash]->word, suffix)))
                {
                  //printf("Suffix entry is first in suffix linked list\n");
                  pretemp->suffix[finalsuffixhash]->total_num_suffix +=1;
                }
                else
                {
                  //printf("Suffix entry is not first in suffix linked list\n");
                  stemp = pretemp->suffix[finalsuffixhash];
                  while(stemp->next != NULL)
                  {
                    if(!(strcmp(stemp->word, suffix)))
                    {
                      break;
                    }
                    stemp = stemp->next;
                  }
                  if(!(strcmp(stemp->word, suffix)))
                  {
                    //printf("Suffix is already an entry\n" );
                    stemp->total_num_suffix +=1;
                  }
                  else
                  {
                    //printf("creating new suffix entry\n" );
                    stemp->next = (suff_entry*)malloc(sizeof(suff_entry));
                    strncpy(stemp->next->word, suffix, strlen(suffix));
                    stemp->next->word[strlen(suffix)] = '\0';
                    stemp->next->total_num_suffix +=1;
                    //temp->next->suffix = NULL;
                    stemp->next->next = NULL;
                  }
              }
              }

            }
            else
            {
              //printf("Prefix is not in the bucket linked list\n");
              pretemp->next = (dict_entry*)malloc(sizeof(dict_entry));
              pretemp = pretemp->next;
              strncpy(pretemp->word, prefix, strlen(prefix));
              pretemp->word[strlen(prefix)] = '\0';
              pretemp->total_num_suffix +=1;
              pretemp->next = NULL;
              pretemp->suffix[finalsuffixhash] = (suff_entry*)malloc(sizeof(suff_entry));
              strncpy(pretemp->suffix[finalsuffixhash]->word, suffix, strlen(suffix));
              pretemp->suffix[finalsuffixhash]->word[strlen(suffix)] = '\0';
              pretemp->suffix[finalsuffixhash]->total_num_suffix +=1;
              //pretemp->suffix[finalsuffixhash]->suffix = NULL;
              pretemp->suffix[finalsuffixhash]->next = NULL;
            }
          }
        }
        else
        {
          totalStart++;
          if((*(startWord[finalsuffixhash].word) == '\0'))
          {
            //printf("Populating empty bucket\n");
            strncpy(startWord[finalsuffixhash].word, suffix, strlen(suffix));
            startWord[finalsuffixhash].word[strlen(suffix)] = '\0';
            startWord[finalsuffixhash].count = startWord[finalsuffixhash].count + 1;
            startWord[finalsuffixhash].next = NULL;
          }
          else if (!(strcmp(startWord[finalsuffixhash].word,suffix)))
          {
            //printf("First bucket entry is the prefix\n");
            startWord[finalsuffixhash].count = startWord[finalsuffixhash].count + 1;
          }

          else
          {
            //printf("First prefix entry is not the prefix\n" );
            startTemp = &startWord[finalsuffixhash];
            while(startTemp->next != NULL)
            {
              if(!(strcmp(startTemp->word, suffix)))
              {
                break;
              }
              startTemp = startTemp->next;

            }
            if(!(strcmp(startTemp->word, suffix)))
            {
              //printf("Prefix entry exists\n");
              startTemp->count++;
            }
            else
            {
              //printf("Prefix is not in the bucket linked list\n");
              startTemp->next = (start_entry*)malloc(sizeof(start_entry));
              startTemp = startTemp->next;
              strncpy(startTemp->word, suffix, strlen(suffix));
              startTemp->word[strlen(suffix)] = '\0';
              startTemp->count +=1;
              startTemp->next = NULL;
            }
          }
        }
        strncpy(prefix, suffix, strlen(suffix));
        prefix[strlen(suffix)] = '\0';
        i = 0;
      }
      buf = fgetc(f);
    }
    fclose(f);
  }

  clock_gettime(CLOCK_REALTIME,&endTime);
  long int nsec, sec;
  sec = endTime.tv_sec - startTime.tv_sec;
  if(endTime.tv_nsec < startTime.tv_nsec)
  {
    sec = sec -1;
    nsec = (1000000000 + endTime.tv_nsec) - startTime.tv_nsec;
  }
  else
  {
    nsec = endTime.tv_nsec - startTime.tv_nsec;
  }
  printf("Time elapsed: %ld seconds and %ld nanoseconds\n", sec, nsec );

  srand(time(NULL));
  double probability;
  double cdf = 1;
  probability = (double)(rand()%10000)/10000;
  //printf("%lf\n", probability);
  for(cnt = 0; cnt < 32768; cnt++)
  {
    startTemp = &startWord[cnt];
    if(startTemp->word[0] == '\0')
    {

    }
    else
    {
      while(startTemp->next != NULL)
      {
        //if((probability > cdf)&&(probability < (double)(cdf + ((double)startTemp->count / (double)totalStart))))
        if(probability > cdf)
        {
          break;
        }
        else
        {
          cdf = (double)(cdf - ((double)startTemp->count / (double)totalStart));
        }
        startTemp = startTemp->next;
      }
      if((probability > cdf))
      {
        printf("%s ", startTemp->word);
        strncpy(prefix, startTemp->word, strlen(startTemp->word));
        prefix[strlen(startTemp->word)] = '\0';
        break;
      }
      else
      {
        cdf = (double)(cdf - ((double)startTemp->count / (double)totalStart));
      }
    }
  }

  int generate = 1;
  int wordlim = 300;
  int wordCount = 1;

  while (generate == 1)
  {
    cdf = 1;
    probability = (double)(rand()%10000)/10000;
    //printf("%f\n",probability);

    MurmurHash3_x86_32(prefix, strlen(prefix), seed, &hash);
    finalhash = hash&(0x00007FFF);

    temp = &dictionary[finalhash];
    while(temp->next != NULL)
    {
      if(!(strcmp(temp->word, prefix)))
      {
        break;
      }
      temp = temp->next;
    }
    //printf("Prefix: %s:\n", temp->word);
    //printf("Possible suffixes: ");
    for(int abc = 0; abc <16; abc++)
    {
      spretemp = temp->suffix[abc];
      if(spretemp != NULL)
      {
        cdf = (double)(cdf - ((double)spretemp->total_num_suffix / (double)temp->total_num_suffix));
        while(spretemp->next != NULL)
        {
          //printf("%f\n",cdf);
          if(probability > cdf)
          {
            break;
          }
          cdf = (double)(cdf - ((double)spretemp->total_num_suffix / (double)temp->total_num_suffix));
          spretemp = spretemp->next;
          //printf("%s ", spretemp->word);
        }
        if((probability > cdf))
        {
          printf("%s ", spretemp->word);
          strncpy(prefix, spretemp->word, strlen(spretemp->word));
          prefix[strlen(spretemp->word)] = '\0';
          break;
        }
        else
        {
          cdf = (double)(cdf - ((double)spretemp->total_num_suffix / (double)temp->total_num_suffix));
        }
      }
    }
    if(prefix[0] == '\0')
    {
      generate = 0;
    }
    else if(wordCount > wordlim)
    {
      generate = 0;
      printf("\n WORD LIMIT REACHED \n");
    }
    else
    {
      wordCount++;
    }
  }
printf("\n");

  return 0;
}
