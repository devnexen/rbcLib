#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#define MENU_VARS	int c = -1; \
			char entry[4];

#define GET_MENU_CHOICE	printf("Enter your choice: "); \
			if(fgets(entry, 4, stdin) != 0) { \
				char * p; \
				c = strtol(entry, & p, 10); \
			} \
			return c;

#endif
