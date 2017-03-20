# Counter Block Code Encryption

#### Parallel Programming Lab Project

## Abstract

The need for encryption of data does not require an explicit justification. Block coding involves encryption of blocks of plaintext data into cipher text blocks. We will be using 64-bit blocks in our encryption algorithm. 

The process uses a 64- bit key known only to the sender and receiver. In counter block coding a consecutively changing counter is used to encrypt the key to form a unique key for each individual block of plaintext. 

This involves encrypting and decrypting the cipher text by encrypting the key using a secret encryption algorithm along with the counter value. The counter value is changed for consecutive blocks of data using some incremental manipulation technique. 

Once the unique 64-bit key is formed it is XOR’ed with the 64-bit plaintext block to form the cipher text block. The cipher text blocks are the joined together in order to form the encrypted data file.

During decryption the same technique is used to extract the unique for each 64-bit block of cipher text, which is then inverted to form the decryption key. This decryption key is XOR’ed with the cipher text blocks to retrieve the original plaintext data file.

The encryption algorithm will involve alternating rounds of substitution and transposition ciphers, like in a Fiestel cipher.

## Algorithm

By _Arjun Rao_

```
Set ctr base valueFor j = 0 to 10    {      Substitute(key)      For i = 0 to 31      {         Key[i] = (key[i] + (ctr % 256)) %256         Circular_shift(ctr, 8)      }      Ctr value warp      Transposition(key)      Shuffle array a[32] with values from 0 to 31      For i = 0 to 31 
  		{         Temp[i] = key[a[i]]         Key[a[i]] = key[i]         Key[i] = temp[i]  		} 	
}
```

