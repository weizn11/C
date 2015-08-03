#ifndef _RC4_H_
#define _RC4_H_

void rc4_crypt(unsigned char *_s,unsigned char *Data, unsigned long data_len);
void rc4_init(unsigned char *s,unsigned char *key, unsigned long key_len);

#endif  //_RC4_H_