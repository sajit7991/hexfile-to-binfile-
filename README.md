 Q: what is intel hex file?
 A: The Intel HEX file is an ASCII text file with lines of text that follow the Intel HEX file format. Each line in an Intel HEX file contains one HEX record.
	These records are made up of hexadecimal numbers that represent machine language code and constant data.
	
	Record Format
	An Intel HEX file is composed of any number of HEX records. Each record is made up of five fields that are arranged in the following format
	:llaaaatt[dd...]cc
	
	Each group of letters corresponds to a different field, and each letter represents a single hexadecimal digit. Each field is composed of at least two hexadecimal digits-which make up a byte-as described below:

    1) : is the colon that starts every Intel HEX record.

    2) ll is the record-length field that represents the number of data bytes (dd) in the record.

    3) aaaa is the address field that represents the starting address for subsequent data in the record.

    4) tt is the field that represents the HEX record type, which may be one of the following:
		00 - data record
		01 - end-of-file record
		02 - extended segment address record
		04 - extended linear address record
		05 - start linear address record (MDK-ARM only)

    5) dd is a data field that represents one byte of data. A record may have multiple data bytes. The number of data bytes in the record must match the number specified by the ll field.

    6) cc is the checksum field that represents the checksum of the record. The checksum is calculated by summing the values of all hexadecimal digit pairs in the record modulo 256 and taking the two's complement.
	
	###Data Records###
	The Intel HEX file is made up of any number of data records that are terminated with a carriage return and a linefeed. Data records appear as follows:
	:10246200464C5549442050524F46494C4500464C33
	|||||||||||                              CC->Checksum
	|||||||||DD->Data
	|||||||TT->Record Type
	|||AAAA->Address
	|LL->Record Length
	:->Colon
	where:
    10 is the number of data bytes in the record.
    2462 is the address where the data are to be located in memory.
    00 is the record type 00 (a data record).
    464C...464C is the data.
    33 is the checksum of the record.
	
	###Extended Linear Address Records ###
	Extended linear address records are also known as 32-bit address records and HEX386 records. These records contain the upper 16 bits (bits 16-31) of the data address. The extended linear address record always has two data bytes and appears as follows:

	:02000004FFFFFC

	where:
    02 is the number of data bytes in the record.
    0000 is the address field. For the extended linear address record, this field is always 0000.
    04 is the record type 04 (an extended linear address record).
    FFFF is the upper 16 bits of the address.
    FC is the checksum of the record and is calculated as
    01h + NOT(02h + 00h + 00h + 04h + FFh + FFh).

	When an extended linear address record is read, the extended linear address stored in the data field is saved and is applied to subsequent records read from the Intel HEX file. The linear address remains effective until changed by another extended address record.
	The absolute-memory address of a data record is obtained by adding the address field in the record to the shifted address data from the extended linear address record. The following example illustrates this process..

	Address from the data record's address field      2462
	Extended linear address record data field     FFFF
												  --------
	Absolute-memory address                       FFFF2462
	
	####Extended Segment Address Records####
	Extended segment address records-also known as HEX86 records-contain bits 4-19 of the data address segment. The extended segment address record always has two data bytes and appears as follows:
	:020000021200EA

	where:
    02 is the number of data bytes in the record.
    0000 is the address field. For the extended segment address record, this field is always 0000.
    02 is the record type 02 (an extended segment address record).
    1200 is the segment of the address.
    EA is the checksum of the record and is calculated as
    01h + NOT(02h + 00h + 00h + 02h + 12h + 00h).

	When an extended segment address record is read, the extended segment address stored in the data field is saved and is applied to subsequent records read from the Intel HEX file. The segment address remains effective until changed by another extended address record.
	The absolute-memory address of a data record is obtained by adding the address field in the record to the shifted-address data from the extended segment address record. The following example illustrates this process.

	Address from the data record's address field     2462
	Extended segment address record data field      1200
												 --------
	Absolute memory address                      00014462
	
	####End-of-File (EOF) Records####
	An Intel HEX file must end with an end-of-file (EOF) record. This record must have the value 01 in the record type field. An EOF record always appears as follows:
	:00000001FF

where:
    00 is the number of data bytes in the record.
    0000 is the address where the data are to be located in memory. The address in end-of-file records is meaningless and is ignored. An address of 0000h is typical.
    01 is the record type 01 (an end-of-file record).
	FF is the checksum of the record and is calculated as
	01h + NOT(00h + 00h + 00h + 01h).
	
	
	
	
	
	
	
	
	
	