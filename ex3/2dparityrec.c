#include <stdio.h>
#include <stdlib.h>
#define MAX 100
#define BITS 8
int data[MAX][BITS];
int rows;
int rec_rowparity[MAX], rec_colparity[BITS];
int calc_rowparity[MAX], calc_colparity[BITS];
char message[MAX];
void recalculateParity()
{
   int i, j, count;
   for (i = 0; i < rows; i++)
   {
      count = 0;
      for (j = 0; j < BITS; j++)
      {
         if (data[i][j] == 1)
            count++;
      }
      calc_rowparity[i] = count % 2;
   }
   for (j = 0; j < BITS; j++)
   {
      count = 0;
      for (i = 0; i < rows; i++)
      {
         if (data[i][j] == 1)
            count++;
      }
      calc_colparity[j] = count % 2;
   }
}
void printMatrix(const char* title)
{
   printf("\n==========================================================\n");
   printf("  RECEIVER MATRIX : %s\n", title);
   printf("===========================================================\n");
   printf(" | Idx | Char | Data Bits | Rec Row P | Cal Row P | Row Status  |\n");
   printf(" +-----+------+-----------+-----------+-----------+-------------+\n");

   for (int i = 0; i < rows; i++)
   {
      printf(" | [%d] |  %c   | ", i, message[i]);
      for (int j = 0; j < BITS; j++)
      {
         printf("%d", data[i][j]);
      }

      int is_row_ok = (rec_rowparity[i] == calc_rowparity[i]);
      printf("  |     %d     |     %d     | %s  |\n",
             rec_rowparity[i], calc_rowparity[i], is_row_ok ? "[Match]      " : "[MISMATCH]");
   }
   printf(" +-----+------+-----------+-----------+-----------+-------------+\n");
   printf("  Received Col Parity : ");
   for (int j = 0; j < BITS; j++) printf("%d", rec_colparity[j]);
   printf("\n  Calculated Col Parity : ");
   for (int j = 0; j < BITS; j++) printf("%d", calc_colparity[j]);
   printf("\n  Column Status Flags   : ");
   int col_errors = 0;
   for (int j = 0; j < BITS; j++)
   {
      if (rec_colparity[j] != calc_colparity[j]) {
         printf("^");
         col_errors++;
      } else {
         printf(" ");
      }
   }
   if (col_errors > 0) {
      printf("  <-- Parity errors detected here!\n");
   } else {
      printf("  <-- All columns matching.\n");
   }
   printf("========================================================\n\n");
}

int main()
{
   int ch;
   FILE *fp = fopen("output.txt", "r");
   if (fp == NULL)
   {
      printf("[ERROR] Could not open output.txt\n");
      return 1;
   }
   fscanf(fp, "%d\n", &rows);
   for (int i = 0; i < rows; i++)
   {
      fscanf(fp, "%c", &message[i]);
   }
   message[rows] = '\0';
   fscanf(fp, "\n");
   for (int i = 0; i < rows; i++)
   {
      for (int j = 0; j < BITS; j++)
      {
         fscanf(fp, "%1d", &data[i][j]);
      }
      fscanf(fp, "%1d\n", &rec_rowparity[i]);
   }
   for (int j = 0; j < BITS; j++)
   {
      fscanf(fp, "%1d", &rec_colparity[j]);
   }
   fclose(fp);

   printf("Received Message: \"%s\" (%d characters)\n", message, rows);
   recalculateParity();
   printMatrix("INITIAL FILE ");

   printf("Would you like to modify any bits? (1 for Yes, 0 for No): ");
   scanf("%d", &ch);
   if (ch == 1)
   {
       int num_bits_to_modify;
       printf("How many bits would you like to modify?: ");
       scanf("%d", &num_bits_to_modify);

       for (int k = 0; k < num_bits_to_modify; k++)
       {
           int target_row, target_col;
           printf("\n--- Modification (%d/%d) ---\n", k + 1, num_bits_to_modify);
           printf("Target Row (0 to %d): ", rows - 1);
           scanf("%d", &target_row);
           printf("Target Bit Column (0 to %d): ", BITS - 1);
           scanf("%d", &target_col);
           if (target_row >= 0 && target_row < rows && target_col >= 0 && target_col < BITS)
           {
              data[target_row][target_col] = !data[target_row][target_col];
              printf("changed value at matrix position [%d][%d].\n", target_row, target_col);
           }
           else
           {
              printf(" Invalid indexes.\n");
           }
       }
       recalculateParity();
       printMatrix("BEFORE MODIFICATION");
   }
   else if (ch == 0)
   {
       printf("\nVerification running on unmodified matrix\n\n");
   }
   else
   {
       printf("\nInvalid choice input\n\n");
   }

   int error_detected = 0;
   for (int i = 0; i < rows; i++)
   {
        if (rec_rowparity[i] != calc_rowparity[i])
        {
           error_detected = 1;
           break;
        }
   }
   if (!error_detected)
   {
       for (int j = 0; j < BITS; j++)
       {
            if (rec_colparity[j] != calc_colparity[j])
            {
               error_detected = 1;
               break;
            }
        }
   }

   printf("=============================================\n");
   printf("               FINAL                      \n");
   printf("=============================================\n");
   if (error_detected)
   {
      printf(" Error discovered!\n");
      printf("Received parity does not match with calculated parity\n");
   }
   else
   {
      printf(" No error!\n");
      printf(" Receiver parity match with calculated parity\n");
   }
   printf("=============================================\n");
   return 0;
}
