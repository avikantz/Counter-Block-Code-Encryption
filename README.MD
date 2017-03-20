# Counter Block Code Encryption

#### Parallel Programming Lab Project

## Abstract



The process uses a 64- bit key known only to the sender and receiver. In counter block coding a consecutively changing counter is used to encrypt the key to form a unique key for each individual block of plaintext. 

This involves encrypting and decrypting the cipher text by encrypting the key using a secret encryption algorithm along with the counter value. The counter value is changed for consecutive blocks of data using some incremental manipulation technique. 

Once the unique 64-bit key is formed it is XOR’ed with the 64-bit plaintext block to form the cipher text block. The cipher text blocks are the joined together in order to form the encrypted data file.



The encryption algorithm will involve alternating rounds of substitution and transposition ciphers, like in a Fiestel cipher.

## Algorithm

By _Arjun Rao_

```
Set ctr base value
  		{
}
```
