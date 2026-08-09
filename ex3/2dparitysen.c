#include <stdio.h>
#include <string.h>
#define MAX 100
#define BITS 8
int data[MAX][BITS];
int rows, rowparity[MAX], colparity[BITS];
char message[MAX];
void charToBinary()
{
   int i,j;
   for(i = 0; i < rows; i++)
   {
      unsigned char ch = message[i];
      for(j = BITS - 1; j >= 0; j--)
      {
         data[i][j] = ch % 2;
         ch /= 2;
      }
   }
}
void rowpar()
{
   int i, j;
   for (i = 0; i < rows; i++)
   {
      int count = 0;
      for (j = 0; j < BITS; j++)
      {
         if (data[i][j] == 1)
            count++;
      }
      rowparity[i] = count % 2;
   }
}
void colpar()
{
   int i, j;
   for (j = 0; j < BITS; j++)
   {
      int count = 0;
      for (i = 0; i < rows; i++)
      {
         if (data[i][j] == 1)
            count++;
      }
      colparity[j] = count % 2;
   }
}
void print2D()
{
   int i, j;
   printf("\n======================================\n");
   printf("       SENDER: 2D PARITY MATRIX   \n");
   printf("=======================================\n");
   printf(" | Idx | Char | Data Bits (0-7) | Row Parity |\n");
   printf(" +-----+------+-----------------+------------+\n");
   for (i = 0; i < rows; i++)
   {
      printf(" | [%d] |  %c   |    ", i, message[i]);
      for (j = 0; j < BITS; j++)
         printf("%d", data[i][j]);
      printf("     |     %d      |\n", rowparity[i]);
   }
   printf(" +-----+------+-----------------+------------+\n");
   printf(" | Column Parity Bits: ");
   for (j = 0; j < BITS; j++)
      printf("%d", colparity[j]);
   printf("        |\n");
   printf("=========================================\n\n");
}

int main()
{
   int i, j;
   FILE *fp;
   printf("Enter message: ");
   scanf("%[^\n]", message);
   rows = strlen(message);
   charToBinary();
   rowpar();
   colpar();
   print2D();
   fp = fopen("output.txt", "w");
   if (fp == NULL)
   {
      printf("[ERROR] Cannot create output.txt\n");
      return 1;
   }
   fprintf(fp, "%d\n", rows);
   for (i = 0; i < rows; i++)
   {
      fprintf(fp, "%c", message[i]);
   }
   fprintf(fp, "\n");
   for (i = 0; i < rows; i++)
   {
      for (j = 0; j < BITS; j++)
         fprintf(fp, "%d", data[i][j]);
      fprintf(fp, "%d\n", rowparity[i]);
   }
   for (j = 0; j < BITS; j++)
      fprintf(fp, "%d", colparity[j]);
   fprintf(fp, "\n");
   fclose(fp);
   printf("Data saved to 'output.txt'\n");
   return 0;
}
