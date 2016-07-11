#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define UINT_MAX 8
#define TAPS 10
#define SIGNAL_LENGTH 50
#define STEP_SIZE 5

//todo:
//figure out scaling factors based on max value of coeffs
//and max value of incomming signal


//Function to print bits
void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    printf("\n");
}

//pushes a new value into an array, 
void PUSH(short int* sig, short int value, int size)
{
	int i;
	short int temp;

	//sig[0] is used as a temporary variable, after loop finishes
	for(i=0; i<size; i++)
	{
		temp = sig[0];
		sig[0] = sig[i];
		sig[i] = temp;
	}
	sig[0]=value;
}

//prints the contents of an array
void print_array(short int* array, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		//detailed print
		printf("ELEMENT %d : %d\n", i, array[i]);
		
		//raw print
		// printf("%d ", array[i]);
	}
	// printf("\n");

}

//Multiply and accumulate
int MAC(short int* sig, unsigned short int* coeff, int size)
{
	int temp, sum = 0;
	int i;

	for(i=0; i<size; i++)
	{
		temp = (int)sig[i] * (int)coeff[i]; /* cast to int */
		temp >>= 8; 						/* adjust to output scale factor */
		sum += temp;  						/* accumulation */
	}	
	sum >>= 16;  //truncating conversion back to 16 bit
	return sum;
}


void INIT(short int* sig, unsigned short int* coeff, short int* incomming_sig, short int* output)
{
	int i;
	//init sig to 0 and coeffs to all 0.5
	for(i=0; i<TAPS; i++)
	{
		sig[i]=0x0;
		coeff[i]=0x8000;	//"0.5" with SF 2^16 see note above
	}
	
	//create a step in input and 0 the output
	for(i=0; i<SIGNAL_LENGTH; i++)
	{
		if(i<STEP_SIZE)
		{
			incomming_sig[i] = 0x4000; //"1" with SF 2^14 see note above
		}
		else
		{
			incomming_sig[i] = 0x0;
		}
		output[i]=0;
	}
}
/*
say that coeff max out at 1 so have SF of 2^16/2^0 = 2^16
so c=0.5 -> C=c*SF = 2^15 = 0x8000
to get back to 0.5 must >> by 16

say that sig max is 4 so SF is 2^16/2^2 = 2^14
so s=1 is 1*2^14 = 0x2000 
to get back to 1 must >> by 14

to get fractional value we must div coeff*sig by 2^(14+16)=2^30

problem is that means our max value for SUM is like ~4

lets max sum is 256 = 2^8 so SF is 2^16/2^8=2^8
lets max sum is 512 = 2^9 so SF is 2^16/2^9=2^7
lets max sum is 1024 = 2^10 so SF is 2^16/2^10=2^6

so we >>8 temp*sig and add it into SUM
*/

int main()
{
	//sig are the elements that are travelling through the FIR filter
	short int sig[TAPS];

	//coeff are the coeffificients of the FIR filter
	unsigned short int coeff[TAPS];

	//incomming_sig are the values getting pushed into the FIR filter
	short int incomming_sig[SIGNAL_LENGTH];

	//output is the accumulated value at the end of the FIR filter
	short int output[SIGNAL_LENGTH];

	//counter variable
	int i;

	// //size is effectively the number of taps
	// int size = sizeof(sig)/sizeof(sig[0]);
	// // printf("sig[] size : %d\n",size);

	INIT(sig,coeff,incomming_sig,output);
	
	/*
	This is the FIR
	MAC: is multiply and accumulate (victim for hardware-ification)
	PUSH: pushes incoming signal into the FIR
	*/
	for(i=0; i<SIGNAL_LENGTH; i++)
	{
		PUSH(sig, incomming_sig[i], TAPS);
		output[i] = MAC(sig, coeff, TAPS);
	}
	

	// printBits(sizeof(sum), &sum);
	float conversion = 0.0;
	float conversion_factor = 1.0*(1<<8)/(1<<14); //.0 is signifigant

	// Print output unscaled and fractional
	for(i=0; i<SIGNAL_LENGTH; i++)
	{
		conversion=(float)(output[i])*conversion_factor;
		printf("OUPUT %d : %d -CONVERSION-> %f\n", i, output[i], conversion);
	}

	return 0;
}