#ifndef CRC_H
#define CRC_H

extern const unsigned short crc_table[];

void startCRC(unsigned short*);
unsigned short updateCRC(unsigned short*, char);

#endif
