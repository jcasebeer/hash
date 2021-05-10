#include <stdio.h>

#define ALEN(a) (sizeof(a)/sizeof(*(a)))

// this only works if out != a or b
int uniq_merge(int *a, int alen, int *b, int blen, int *out)
{
    int len = 0;
    int ac = 0;
    int bc = 0;
    while(ac<alen && bc<blen){
        if (a[ac] < b[bc])
            out[len++] = a[ac++];
        else
        if (a[ac] > b[bc])
            out[len++] = b[bc++];
        else {
            out[len++] = a[ac];
            ac++;
            bc++;
        }
    }
    while(ac<alen)
        out[len++] = a[ac++];
    while(bc<blen)
        out[len++] = b[bc++];
    return len;
}

int merge_sorted_into(int *in, int inlen, int *out, int outlen)
{
    int end = 0;
    int inc = 0;
    int outc = 0;
    // iterate forward in the arrays to find duplicates
    // so that we know the total number of unique elements
    while(inc<inlen && outc<outlen)
    {
        if (in[inc] < out[outc]) 
            inc++;
        else if (in[inc] > out[outc])
            outc++;
        else {
            outc++;
            inc++;
        }
        end++;
    }
    end+=(inlen-inc);
    end+=(outlen-outc);
    int len=end;

    // now iterate backwards so that we can combine the already sorted arrays
    // without scribbling over
    inc = inlen - 1;
    outc = outlen - 1;
    while(inc >= 0 && outc >= 0) {
        if (in[inc] > out[outc]) 
            out[--end] = in[inc--]; 
        else if (in[inc] < out[outc]) 
            out[--end] = out[outc--];
        else {
            out[--end] = out[outc--];
            inc--;
        }
    }
    while(inc>=0)
        out[--end] = in[inc--]; 
    while(outc>=0)
        out[--end] = out[outc--];
    return len;
}

int main()
{
    int arr0[] = {1,2,3,8,100,500,700};
    int arr1[] = {2,3,7,8,101,501,502,503,504,700,701};
    int arr2[] = {7,8,9,10,20,701};

    int arr3[] = {1,3,5,77};
    int arr4[] = {5,9,20};
    int arr5[] = {20, 69, 70};

    int *arrays[] = {arr0, arr1, arr2, arr3, arr4, arr5};
    size_t array_counts[] = {ALEN(arr0), ALEN(arr1), ALEN(arr2), ALEN(arr3), ALEN(arr4), ALEN(arr5)};
    int out[64] = {0};
    int i;
    int len=0;

    
    for(i = 0; i<ALEN(arrays); i++) {
        len = merge_sorted_into(arrays[i], array_counts[i], out, len);
    }


    for(i = 0; i<len; i++)
        printf("%d ", out[i]);

    printf("\n");
    return 0;
}
