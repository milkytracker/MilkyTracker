/*----------------------------------------------------------------------*/
/*	header.c (from lharc.c)	-- header manipulate functions		*/
/*	Original					by Y.Tagawa	*/
/*	modified  Dec 16 1991				by M.Oki	*/
/*----------------------------------------------------------------------*/

static int calc_sum(char *p, int len)
{
	int sum;
	for (sum = 0; len; len --) sum += *p++;
	return sum & 0xff;
}

/*----------------------------------------------------------------------*/
/*			build header functions										*/
/*----------------------------------------------------------------------*/

bool CLhaArchive::get_header(pp_uint32 &fp, LzHeader *hdr)
//----------------------------------------------------
{
	int header_size;
	int name_length;
	char data[LZHEADER_STRAGE];
	char dirname[FILENAME_LENGTH];
	int dir_length = 0;
	int checksum;
	unsigned char uc;

	memset(hdr, 0, sizeof(LzHeader));
	if (!lharead(&uc, 1, 1, fp)) return false;
	header_size = uc;
	if ((!header_size) || (header_size > LZHEADER_STRAGE - I_HEADER_CHECKSUM)) return false;
	if (lharead(data + I_HEADER_CHECKSUM, 1, header_size - 1, fp) < header_size - 1) return false;
	
	setup_get(data + I_HEADER_LEVEL);
	hdr->header_level = get_byte();
	if (hdr->header_level != 2 &&
		lharead(data + header_size, sizeof (char), 2, fp) < 2)
	{
		return false;
	}

	setup_get (data + I_HEADER_CHECKSUM);
	checksum = get_byte();

	hdr->header_size = header_size;
	memcpy(hdr->method, data + I_METHOD, METHOD_TYPE_STRAGE);
	setup_get (data + I_PACKED_SIZE);
	hdr->packed_size	= get_longword();
	if ((unsigned long)hdr->packed_size > m_dwStreamLen) return false;
	hdr->original_size	= get_longword();
	if ((unsigned long)hdr->original_size > 0x4000000) return false;
	hdr->last_modified_stamp = get_longword();
	hdr->attribute	= get_byte();

	if ((hdr->header_level = get_byte ()) != 2)
	{
		if (calc_sum(data + I_METHOD, header_size) != checksum)
		{
		#ifdef LHADEBUG
			Log("WARNING: Archive Checksum error\n");
			Log("header_level = %d\n", hdr->header_level);
			Log("header_size  = %d\n", hdr->header_size);
			Log("packed_size = %d\n", hdr->packed_size);
			Log("original_size = %d\n", hdr->original_size);
		#endif
			if ((hdr->header_level > 3)
			 || (hdr->packed_size > hdr->original_size)) return false;
		}
		name_length	= get_byte();
		for (int i = 0; i < name_length; i ++) hdr->name[i] = (char)get_byte();
		hdr->name[name_length] = '\0';
	} else
	{
		name_length = 0;
	}

	// defaults for other type
	hdr->unix_mode	= UNIX_FILE_REGULAR | UNIX_RW_RW_RW;

	// EXTEND FORMAT
	if (header_size - name_length >= 24)
    {
		hdr->crc = get_word();
		hdr->extend_type = get_byte();
		hdr->has_crc = true;
	} else
	// Generic with CRC
	if (header_size - name_length == 22)
	{
		hdr->crc = get_word ();
		hdr->extend_type = EXTEND_GENERIC;
		hdr->has_crc = true;
	} else
	// Generic no CRC
	if (header_size - name_length == 20)
	{
		hdr->extend_type = EXTEND_GENERIC;
		hdr->has_crc = false;
	} else
	{
		return false;
	}

	if (hdr->extend_type == EXTEND_UNIX && hdr->header_level == 0)
	{
		hdr->minor_version = get_byte();
		hdr->unix_last_modified_stamp = get_longword();
		hdr->unix_mode = get_word();
		get_word(); // hdr->unix_uid = get_word();
		get_word(); // hdr->unix_gid = get_word();
		return true;
	}

	if (hdr->header_level > 0)
	{
		// Extend Header
		if (hdr->header_level != 2) setup_get(data + hdr->header_size);
		char *ptr = get_ptr;
		while((header_size = get_word()) != 0)
		{
			if (hdr->header_level != 2 &&
				((data + LZHEADER_STRAGE - get_ptr < header_size) ||
				(lharead(get_ptr, sizeof(char), header_size, fp) < header_size)))
			{
				return false;
			}
			switch (get_byte())
			{
			// Header CRC
			case 0:
				setup_get(get_ptr + header_size - 3);
				break;
			// File Name
			case 1:
				{
					if (header_size >= 256+3) return false;
					for (int i = 0; i < header_size - 3; i++) hdr->name[i] = (char)get_byte();
					hdr->name[header_size - 3] = '\0';
				}
				break;
			// Directory
			case 2:
				{
					if (header_size >= FILENAME_LENGTH+3) return false;
					for (int i = 0; i < header_size - 3; i++) dirname[i] = (char)get_byte ();
					dirname[header_size - 3] = '\0';
					dir_length = header_size-3;
				}
				break;
			// MS-DOS attribute
			case 0x40:
				if (hdr->extend_type == EXTEND_MSDOS ||
					hdr->extend_type == EXTEND_HUMAN ||
					hdr->extend_type == EXTEND_GENERIC)
				hdr->attribute = (unsigned char)get_word();
				break;
			// UNIX Permission
			case 0x50:
				if (hdr->extend_type == EXTEND_UNIX)
					hdr->unix_mode = get_word();
				break;
			// UNIX gid and uid
			case 0x51:
				if (hdr->extend_type == EXTEND_UNIX)
				{
					get_word(); //hdr->unix_gid = get_word();
					get_word(); //hdr->unix_uid = get_word();
				}
				break;
			// UNIX last modified time
			case 0x54:
				if (hdr->extend_type == EXTEND_UNIX) hdr->unix_last_modified_stamp = get_longword();
				break;
			// Other headers
			default:
				setup_get(get_ptr + header_size - 3);
				break;
			}
		}
		if (hdr->header_level != 2 && get_ptr - ptr != 2)
		{
			hdr->packed_size -= get_ptr - ptr - 2;
			hdr->header_size += get_ptr - ptr - 2;
		}
	}
	if (dir_length)
    {
		strcat(dirname, hdr->name);
		strcpy(hdr->name, dirname);
		name_length += dir_length;
	}
	return true;
}

