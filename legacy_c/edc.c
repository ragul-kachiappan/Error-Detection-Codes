#include<stdio.h>
#include<conio.h>
#include<string.h>


//****************************CHECKSUM***************************
void checksum()
{
int k,n;
printf("\nEnter the number of blocks in data:");
scanf("%d",&k);
printf("\nEnter block size:");
scanf("%d",&n);
int sdata[k+1][n];
int rdata[k+1][n];
int sum[n],rsum[n],carry=0;
printf("\nEnter the sequence of data to be transmitted:");
for(int i=0;i<k;i++)
{
  printf("\nBlock-%d:",i+1);
  for(int j=0;j<n;j++)
    scanf("%d",&sdata[i][j]);
    
}


for(int i=0;i<k-1;i++)
{
if(i==0)
{
for(int j=n-1;j>=0;j--)
{
    sum[j]=sdata[i][j]^sdata[i+1][j]^carry;
    carry=(sdata[i][j]&sdata[i+1][j])|(sdata[i+1][j]&carry)|(carry&sdata[i][j]);
}

}
else
{

for(int j=n-1;j>=0;j--)
{
    int prevsum=sum[j];
    sum[j]=sum[j]^sdata[i+1][j]^carry;
    carry=(prevsum&sdata[i+1][j])|(sdata[i+1][j]&carry)|(carry&prevsum);
}
}
if(carry==1)
{   carry=0;
    int temp[n];
    for(int m=0;m<n-1;m++)
        temp[m]=0;
    temp[n-1]=1;

    for(int j=n-1;j>=0;j--)
    {
    int prevsum=sum[j];
    sum[j]=sum[j]^temp[j]^carry;
    carry=(prevsum&temp[j])|(temp[j]&carry)|(carry&prevsum);
    }

}




}
for(int i=0;i<n;i++)
        sum[i]=!sum[i];
printf("\nChecksum value: ");
for(int i=0;i<n;i++)
   {
    printf("%d ",sum[i]);
    sdata[k][i]=sum[i];
   }
printf("\nData to be transmitted: ");
for(int i=0;i<k+1;i++)
{
    for(int j=0;j<n;j++)
    {printf("%d",sdata[i][j]);
    }
    printf("   ");
}

printf("\nEnter the sequence of data received: ");
for(int i=0;i<k+1;i++)
{
  printf("\nBlock-%d:",i+1);
  for(int j=0;j<n;j++)
    scanf("%d",&rdata[i][j]);
}
carry=0;
for(int i=0;i<k;i++)
{

if(i==0)
{
for(int j=n-1;j>=0;j--)
{
    rsum[j]=rdata[i][j]^rdata[i+1][j]^carry;
    carry=(rdata[i][j]&rdata[i+1][j])|(rdata[i+1][j]&carry)|(carry&rdata[i][j]);
}
}
else
{

for(int j=n-1;j>=0;j--)
{
    int prevsum=rsum[j];
    rsum[j]=rsum[j]^rdata[i+1][j]^carry;
    carry=(prevsum&rdata[i+1][j])|(rdata[i+1][j]&carry)|(carry&prevsum);
}
}
if(carry==1)
{   carry=0;
    int temp[n];
    for(int m=0;m<n-1;m++)
        temp[m]=0;
    temp[n-1]=1;

    for(int j=n-1;j>=0;j--)
    {
    int prevsum=rsum[j];
    rsum[j]=rsum[j]^temp[j]^carry;
    carry=(prevsum&temp[j])|(temp[j]&carry)|(carry&prevsum);
    }

}

}
for(int i=0;i<n;i++)
        rsum[i]=!rsum[i];
printf("\nchecksum of received data: ");
int check=0;
for(int i=0;i<n;i++)
 {
    printf("%d ",rsum[i]);
    if(rsum[i]!=0)
        check=1;
 }
if(check==1)
   printf("\nReceived data checksum is NON ZERO, ERROR DETECTED.");
else
   printf("\nReceived data checksum is ZERO, NO ERROR IN DATA.");
}


//****************************LRC***************************
void lrc()
{
int nob,bsize;
printf("\nEnter the number of blocks in data:");
scanf("%d",&nob);
printf("\nEnter block size:");
scanf("%d",&bsize);
int sdata[nob+1][bsize];
int rdata[nob+1][bsize];
int LRC[bsize];
printf("\nEnter the sequence of data to be transmitted:");
for(int i=0;i<nob;i++)
{
  printf("\nBlock-%d:",i+1);
  for(int j=0;j<bsize;j++)
    scanf(" %d",&sdata[i][j]);
}
for(int i=0;i<bsize;i++)
{
    LRC[i]=0;
    for(int j=0;j<nob;j++)
        {
            LRC[i]^=sdata[j][i];
        }
        sdata[nob][i]=LRC[i];
}
printf("\nCalculated parity block:");
for(int i=0;i<bsize;i++)
printf("%d",LRC[i]);
printf("\n");
printf("\nTransmitted data:\n");
for(int i=0;i<nob+1;i++)
{
  for(int j=0;j<bsize;j++)
    printf("%d",sdata[i][j]);
    printf(" ");
}
printf("\nEnter the sequence of data received:");
for(int i=0;i<nob+1;i++)
{
  printf("\nByte-%d:",i+1);
  for(int j=0;j<bsize;j++)
    scanf(" %d",&rdata[i][j]);
}
for(int i=0;i<bsize;i++)
{
    LRC[i]=0;
    for(int j=0;j<nob;j++)
        {
            LRC[i]^=rdata[j][i];
        }
}
printf("\nCalculated parity block:");
for(int i=0;i<bsize;i++)
printf("%d",LRC[i]);
printf("\n");
printf("\nLRC from received data:");
int check=0;
for(int i=0;i<bsize;i++)
{
    printf("%d",rdata[nob][i]);
    if(LRC[i]!=rdata[nob][i])
    {
        check=1;
    }
}
printf("\n");
if(check==0)
    printf("NO ERROR in received data");
else
    printf("ERROR DETECTED");


}


//****************************VRC***************************
void vrc(){
int bsize,nob;
printf("\nEnter the no of data blocks:");
scanf("%d",&nob);
printf("\nEnter the block size:");
scanf("%d",&bsize);
int sdata[nob][bsize+1];
int rdata[nob][bsize+1];
int LRC[nob];
printf("\nEnter the sequence of data to be transmitted:");
for(int i=0;i<nob;i++)
{
  printf("\nBlock-%d:",i+1);
  for(int j=0;j<bsize;j++)
    scanf("%d",&sdata[i][j]);
}
for(int i=0;i<nob;i++)
{
    LRC[i]=0;
    for(int j=0;j<bsize;j++)
        {
            LRC[i]^=sdata[i][j];
        }
    sdata[i][bsize]=LRC[i];
}
printf("\nCalculated parity block:");
for(int i=0;i<nob;i++)
printf("%d",LRC[i]);
printf("\n");
printf("\nTransmitted data:\n");
for(int i=0;i<nob;i++)
{
  for(int j=0;j<bsize+1;j++)
    printf("%d",sdata[i][j]);
    printf(" ");
}

printf("\nEnter the sequence of data on received:");
for(int i=0;i<nob;i++)
{
  printf("\nByte-%d:",i+1);
  for(int j=0;j<bsize+1;j++)
    scanf("%d",&rdata[i][j]);
}
int count[nob],check=0;
for(int i=0;i<nob;i++)
{
  count[i]=0;
  for(int j=0;j<bsize+1;j++)
    {
      if(rdata[i][j]==1)
            count[i]+=1;
    }
}
for(int i=0;i<nob;i++)
{

    if(count[i]%2!=0)
    {
      printf("-->odd number of ones detected at block %d.",i+1);
      check=1;
    }
}
if(check==0)
    printf("\nNO ERROR in received data");
else
    printf("\nERROR DETECTED");
}


//****************************CRC***************************
void crc()
{
char databuffer[100],keybuffer[100],rcodebuffer[100];
int dataword[100],key[100],codeword[100],rcodeword[100],dsize,ksize,csize,rcsize;
int remainder[100],r_remainder[100];
printf("\nEnter the dataword:");
scanf("%s",databuffer);
printf("\nEnter the key:");
scanf("%s",keybuffer);
dsize=strlen(databuffer);
ksize=strlen(keybuffer);
for(int i=0;i<dsize;i++)
    dataword[i]=databuffer[i]-'0';
for(int i=0;i<ksize;i++)
    key[i]=keybuffer[i]-'0';
csize=dsize+(ksize-1);
for(int i=0;i<csize;i++)
{
    if(i<dsize)
    codeword[i]=dataword[i];
    else
    codeword[i]=0;
}
printf("\ndataword: ");
for(int i=0;i<dsize;i++)
    printf("%d ",dataword[i]);
printf("\nkey: ");
for(int i=0;i<ksize;i++)
    printf("%d ",key[i]);
printf("\ncodeword: ");
for(int i=0;i<csize;i++)
    printf("%d ",codeword[i]);



int temp[ksize],quotient[dsize];
for(int i=0;i<dsize;i++)
{
//printf("\nIteration %d:",i+1);
if(i==0)
{

    if(codeword[i]==0)
    {
        for(int j=0;j<ksize;j++)
        temp[j]=codeword[j]^0;
        quotient[i]=0;
    }
    else
    {
        for(int j=0;j<ksize;j++)
        temp[j]=codeword[j]^key[j];
        quotient[i]=1;
    }
}

else
{
    for(int k=0;k<ksize-1;k++)
        temp[k]=temp[k+1];
    temp[ksize-1]=codeword[i+ksize-1];
    /*printf("\nupdated temp:");
    for(int k=0;k<ksize;k++)
        printf("%d",temp[k]);*/
    if(temp[0]==0)
    {
        for(int j=0;j<ksize;j++)
        temp[j]=temp[j]^0;
        quotient[i]=0;
    }
    else
    {
        for(int j=0;j<ksize;j++)
        temp[j]=temp[j]^key[j];
        quotient[i]=1;
    }
}
/*printf("\nvalue of temp:");
for(int k=0;k<ksize;k++)
    printf("%d",temp[k]);
printf("\n");*/
}
printf("\nGenerated quotient: ");
for(int i=0;i<dsize;i++)
    printf("%d",quotient[i]);
for(int i=0;i<ksize-1;i++)
    remainder[i]=temp[i+1];

for(int i=dsize,j=0;i<csize;i++,j++)
    codeword[i]=remainder[j];
printf("\nGenerated CRC remainder: ");
for(int i=0;i<ksize-1;i++)
    printf("%d",remainder[i]);

printf("\nGenerated codeword: ");
for(int i=0;i<csize;i++)
    printf("%d",codeword[i]);

printf("\nEnter the received codeword: ");
scanf("%s",rcodebuffer);
rcsize=strlen(rcodebuffer);
for(int i=0;i<rcsize;i++)
    rcodeword[i]=rcodebuffer[i]-'0';
printf("\ncodeword: ");
for(int i=0;i<csize;i++)
    printf("%d ",rcodeword[i]);

for(int i=0;i<dsize;i++)
{
//printf("\nIteration %d:",i+1);
if(i==0)
{

    if(rcodeword[i]==0)
    {
        for(int j=0;j<ksize;j++)
        temp[j]=rcodeword[j]^0;
        quotient[i]=0;
    }
    else
    {
        for(int j=0;j<ksize;j++)
        temp[j]=rcodeword[j]^key[j];
        quotient[i]=1;
    }
}
else
{
    for(int k=0;k<ksize-1;k++)
        temp[k]=temp[k+1];
    temp[ksize-1]=rcodeword[i+ksize-1];
    /*printf("\nupdated temp:");
    for(int k=0;k<ksize;k++)
        printf("%d",temp[k]);*/
    if(temp[0]==0)
    {
        for(int j=0;j<ksize;j++)
        temp[j]=temp[j]^0;
        quotient[i]=0;
    }
    else
    {
        for(int j=0;j<ksize;j++)
        temp[j]=temp[j]^key[j];
        quotient[i]=1;
    }
}
}
int check=0;
printf("\nCRC of received codeword: ");
for(int i=0;i<ksize-1;i++)
   {
    r_remainder[i]=temp[i+1];
    printf("%d ",r_remainder[i]);
    if(r_remainder[i]!=0)
        check=1;
   }
if(check==1)
    printf("\nCRC NON-ZERO VALUE, ERROR DETECTED.");
else
    printf("\nREMAINDER ZERO, NO ERROR IN RECEIVED CODEWORD.");

}
void main()
{

    int choice;
    char ch='y';
    while(ch=='y'||ch=='Y')
    {
    printf("\nERROR DETECTION CODES:");
    printf("\n 1. CHECKSUM");
    printf("\n 2. VRC");
    printf("\n 3. LRC");
    printf("\n 4. CRC");
    printf("\n ENTER YOUR CHOICE: ");
    scanf("%d",&choice);
    switch(choice)
    {
    case 1: checksum();
    break;
    case 2: vrc();
    break;
    case 3: lrc();
    break;
    case 4: crc();
    break;
    case 5: printf("\nINVALID CHOICE.TRY AGAIN.");
    break;
    }
    printf("\nDo you want to try again(Y/N):");
    scanf(" %c",&ch);
    getchar();
    
    }

}
