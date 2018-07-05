#define MAX_LOCKER_ITEMS 50  

struct locker_info{
    char filename[MAX_STRING_LENGTH];
    char rented;
    char opened;
    int  day;
    int  month;
    int  year;
	long value;
    long insurance;
    struct obj_file_elem items[MAX_LOCKER_ITEMS+1];
	int  num_items;
};

