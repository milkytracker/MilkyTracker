static pp_int32 ASCIISTEP16_channel(pp_uint8 ascii)
{
	pp_int32 number = -1;
	switch (ascii)
	{
		// ASCIISTEP16 standard (https://gist.github.com/coderofsalvation/8d760b191f4bb5465c8772d5618e5c4b)
		case '1': number = 0; break;
		case '2': number = 1; break;
		case '3': number = 2; break;
		case '4': number = 3; break;
		case '5': number = 4; break;
		case '6': number = 5; break;
		case '7': number = 6; break;
		case '8': number = 7; break;
		case '9': number = 8; break;
		case '0': number = 9; break;
		case '-': number = 10; break;
		case '=': number = 11; break;
	}
	return number;
}

static pp_int32 ASCIISTEP16(pp_uint8 ascii,pp_uint16 bar)
{
	pp_int32 number = -1;
	switch (ascii)
	{
		// ASCIISTEP16 standard (https://gist.github.com/coderofsalvation/8d760b191f4bb5465c8772d5618e5c4b)
		case 'Q': number = 0; break;
		case 'W': number = 1; break;
		case 'E': number = 2; break;
		case 'R': number = 3; break;
		case 'T': number = 4; break;
		case 'Y': number = 5; break;
		case 'U': number = 6; break;
		case 'I': number = 7; break;
		case 'A': number = 8; break;
		case 'S': number = 9; break;
		case 'D': number = 10; break;
		case 'F': number = 11; break;
		case 'G': number = 12; break;
		case 'H': number = 13; break;
		case 'J': number = 14; break;
		case 'K': number = 15; break;
	}
	return number == -1 ? -1 : number + (bar*16);
}
