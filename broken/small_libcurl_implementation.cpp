/**
 ** Simple curl/libcurl implementation in cpp.
 **
 ** date: 16 JUN 2025
 ** context: missing free calls. probably from libcurl,
 **	invalid use of it of course.
 */

#include <print>
#include <cstring>
#include <bits/stdc++.h>
#include <curl/curl.h>
#include <getopt.h>
using namespace std;

namespace Network {
	class Connect;

	namespace {
		typedef enum {
			NONE, ERR, GOOD, EXIT
		} Status;
		struct Memory_Chunk {
			string buf;
			char* m;
			size_t size;
		} chunk;

		vector <Connect*> all_active;

		size_t write(void* contents, size_t size, size_t nmemb, string userp) {
			size_t realsize = size * nmemb;
			userp.append((char*)contents, size*nmemb);

			return realsize;
		}
	}

	class Connect {
	private:
#		define NETWORK_RETURN		if (code != GOOD) { fputs("some error occured", stderr); return 1;};
		Status code = NONE;
		char** url=NULL;
		CURL* handle;
		CURLcode res;
		FILE* fp = NULL;

	public:
		Connect() {
		}

		int init(char** u) {
			if (code == NONE)
				all_active.push_back(this);

			// TODO: check if u is NULL
			url = u;
			fprintf(stderr, "URL: '%s'\n", *url);
			if ((*url) == NULL || url == NULL) {
				code = ERR;
				return 1;
			}

			// TODO: check if already ran
			chunk.m = (char*)malloc(1);
			chunk.size = 0;

			handle = curl_easy_init();
			code = GOOD;
			return 0;
		}

		int run(void) {
			NETWORK_RETURN

				if (handle) {
					curl_easy_setopt(handle,
							CURLOPT_URL, (*url));
					curl_easy_setopt(handle,
							CURLOPT_FOLLOWLOCATION, 1L);
					curl_easy_setopt(handle,
							CURLOPT_WRITEFUNCTION, write);
					curl_easy_setopt(handle,
							CURLOPT_WRITEDATA, (void *)&chunk);
					curl_easy_setopt(handle,
							CURLOPT_USERAGENT, "libcurl-agent/1.0");

					res = curl_easy_perform(handle);


					if (res != CURLE_OK) {
						fprintf(stderr, "error: %s\n", curl_easy_strerror(res));
					} else {
						fprintf(stderr, "Size: %u\n", (unsigned int)chunk.size);
					}

					curl_easy_cleanup(handle);
					return 0;
				}
			return 1;
		}

		int end(void) {
			curl_easy_cleanup(handle);
			code = EXIT;
			if (fp != NULL)
				fclose(fp);
			return 0;
		}

		int print(void) {
			if (fp == NULL)
				fp = stdout;
			fprintf(fp, "%s", chunk.buf.c_str());

			return 0;
		}
		int set_output(char* name) {
			if (name == NULL)
				fp = stdout;
			else
				fp = fopen(name, "w");

			return 0;
		}
	};

	void init(void) {
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}
	void end_all(void) {
		for (int x=0; x<all_active.size(); x++) {
			all_active[x]->end();
		}
		curl_global_cleanup();
		return;
	}
}

void print_help(void) {
		fprintf(stderr, "usage: try './%s [[flags|input]] ... url'\n", __FILE__);
		exit(1);
}

int main(int argc, char* argv[]) {
	Network::init();
	Network::Connect net;

	if (argc < 2) {
		fprintf(stderr, "Not Enough Arguments: %d\n", argc);
		print_help();
	}

	struct option longopts[] = {
		{"help",	no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	bool init = false;
	int opt = 0;
	while ((opt = getopt_long(argc, argv, "ho:", longopts, NULL)) != -1) {
		switch (opt) {
		case '?':
			fprintf(stderr, "unknown opt '%s'\n", optarg);
		case 'h':
			print_help();
			return 0;
		case 'o':
			net.set_output((char*)optarg);
			break;
		default:
			init = true;
			net.init(&argv[optind]);
			break;
		}
	}

	if (init == false)
		net.init(&argv[optind]);

	net.run();
	net.print();
	Network::end_all();
}
