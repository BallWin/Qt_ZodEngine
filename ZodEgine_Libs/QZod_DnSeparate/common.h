#ifndef COMMON_H
#define COMMON_H

#include "qzod_dnseparate_global.h"

namespace COMMON
{
	//structs
    class QZOD_DNSEPARATESHARED_EXPORT xy_struct
	{
	public:
		xy_struct() { x=y=0; }
		xy_struct(int x_, int y_) { x=x_; y=y_; }

		int x, y;
	};

	QZOD_DNSEPARATESHARED_EXPORT void split(char *dest, char *message, char split, int *initial, int d_size, int m_size);
	QZOD_DNSEPARATESHARED_EXPORT void clean_newline(char *message, int size);
	QZOD_DNSEPARATESHARED_EXPORT void lcase(char *message, int m_size);
	QZOD_DNSEPARATESHARED_EXPORT void lcase(string &message);
	QZOD_DNSEPARATESHARED_EXPORT double current_time();
	QZOD_DNSEPARATESHARED_EXPORT void create_folder(char *foldername);
	QZOD_DNSEPARATESHARED_EXPORT void uni_pause(int m_sec);
	QZOD_DNSEPARATESHARED_EXPORT char *wtoc_s(const wchar_t *input);
	QZOD_DNSEPARATESHARED_EXPORT char *wtoc(const wchar_t *input, char *dest, int size);
	QZOD_DNSEPARATESHARED_EXPORT wchar_t *ctow_s(const char *input);
	QZOD_DNSEPARATESHARED_EXPORT wchar_t *ctow(const char *input, wchar_t *dest, int size);
	QZOD_DNSEPARATESHARED_EXPORT void print_dump(char *message, int size, char *name);
	QZOD_DNSEPARATESHARED_EXPORT bool points_within_distance(int x1, int y1, int x2, int y2, int distance);
	QZOD_DNSEPARATESHARED_EXPORT bool points_within_area(int px, int py, int ax, int ay, int aw, int ah);
	QZOD_DNSEPARATESHARED_EXPORT bool good_user_char(int c);
	QZOD_DNSEPARATESHARED_EXPORT bool good_user_string(const char *message);
	QZOD_DNSEPARATESHARED_EXPORT void printd_reg(char *message);
	QZOD_DNSEPARATESHARED_EXPORT string data_to_hex_string(unsigned char *data, int size);
	QZOD_DNSEPARATESHARED_EXPORT bool file_can_be_written(char *filename);
	QZOD_DNSEPARATESHARED_EXPORT vector<string> directory_filelist(string foldername);
	QZOD_DNSEPARATESHARED_EXPORT void parse_filelist(vector<string> &filelist, string extension);
	QZOD_DNSEPARATESHARED_EXPORT bool sort_string_func (const string &a, const string &b);

	//inline functions...
    inline bool isz(float num) { return (num < 0.00001f && num > -0.00001f); };
	inline bool isz(double num) { return (num < 0.00001 && num > -0.00001); };

    inline bool is1(float num) { return (num < 1.00001f && num > 0.99999f); };
	inline bool is1(double num) { return (num < 1.00001 && num > 0.99999); };

	inline void swap(int &a, int &b)
	{
		int c;

		c = a;
		a = b;
		b = c;
	}

	inline double frand() { return (rand()%10001) / 10000.0; }
};

#endif
