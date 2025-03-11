#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GET_SIZE(size) (((size[0] & 0xFF) << 24) | \
                        ((size[1] & 0xFF) << 16) | \
                        ((size[2] & 0xFF) << 8)  | \
                         (size[3] & 0xFF))

#define FIX_SIZE(size, new_len)  size[0] = (new_len >> 24) & 0xFF; \
                                 size[1] = (new_len >> 16) & 0xFF; \
                                 size[2] = (new_len >> 8)  & 0xFF; \
                                 size[3] = new_len & 0xFF;

struct reader {
    char *src_fname;
    FILE *fptr_src;
};
void copying(FILE *fptr, FILE *fptr1)
{
    char buffer[4096]; 
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fptr1)) > 0)
    {
        fwrite(buffer, 1, bytes, fptr);
    }
}

void view_mp3_tag(char *filename) {
    FILE *file;
    char id3[4] = {0};
    unsigned char vers[2];

    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }

    fread(id3, 1, 3, file);
    if (strcmp(id3, "ID3") != 0) {
        printf("No ID3 tag found in the file.\n");
        fclose(file);
        return;
    }

    fread(vers, 1, 2, file);
    if (vers[0] != 3) {
        printf("Unsupported ID3 version: %d.%d\n", vers[0], vers[1]);
        fclose(file);
        return;
    }
    int len = strlen(filename);
    if (len < 4 || strcmp(&filename[len - 4], ".mp3") != 0) {
        printf("Error: File extension is not .mp3\n");
        fclose(file);
        return;
    }

    fseek(file, 10, SEEK_SET);
    char frame_id[5];
    unsigned char frame_size[4];
    unsigned char frame_flags[2];
    unsigned int frame_data_size;
    char *frame_data;
    printf("-------------SELECTED VIEW DETAILS----------------\n\n");
    printf("----------------------------------------------------\n");
    printf("        MP3 TAG READER AND EDITOR FOR ID3v2         \n");
    printf("----------------------------------------------------\n");
    while (fread(frame_id, 1, 4, file) == 4)
    {

        frame_id[4] = '\0';
        fread(frame_size, 1, 4, file);

        frame_data_size = GET_SIZE(frame_size);
        fread(frame_flags, 1, 2, file);
    	frame_data = (char *)malloc(frame_data_size + 1);
    	if (frame_data == NULL)
    	{
        	printf("Error allocating memory for frame data.\n");
        	fclose(file);
        	return;
    	}	

        fread(frame_data, 1, frame_data_size, file);
        frame_data[frame_data_size] = '\0';

        if (strcmp(frame_id, "TIT2") == 0) {
            printf("Title: %s\n", frame_data + 1);
        } else if (strcmp(frame_id, "TPE1") == 0) {
            printf("Artist: %s\n", frame_data + 1);
        } else if (strcmp(frame_id, "TALB") == 0) {
            printf("Album: %s\n", frame_data + 1);
        } else if (strcmp(frame_id, "TYER") == 0) {
            printf("Year: %s\n", frame_data + 1);
        } else if (strcmp(frame_id, "TCON") == 0) {
            printf("Content: %s\n", frame_data + 1);
        } else if (strcmp(frame_id, "TCOM") == 0) {
            printf("Composer: %s\n", frame_data + 1);
        }

        free(frame_data);

    }
    printf("----------------------------------------------------\n");
    printf("----------DETAILS DISPLAYED SUCCESSFULLY------------\n");
    fclose(file);
}

void edit_mp3_tag(char *argv[], char *filename)
{
    
    char frame_id[5];
    unsigned char frame_size[4];
    unsigned char frame_flags[2];
    unsigned int frame_data_size;
    char *frame_data;
    char *new_value = argv[3];
    int new_len = strlen(new_value)+1;

    int modified = 0; // To check if the tag is modified

    FILE *fptr1 = fopen(filename, "r");
    if (fptr1 == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }
    FILE *fptr2 = fopen("newfile.mp3", "w");
    if (fptr2 == NULL) {
        printf("Error: Could not create output file\n");
        fclose(fptr2);
        return;
    }
    char id3[4] = {0};
    unsigned char vers[2];

    fread(id3, 1, 3, fptr1);
    if (strcmp(id3, "ID3") != 0) {
        printf("No ID3 tag found in the file.\n");
        fclose(fptr1);
        return;
    }

    fread(vers, 1, 2, fptr1);
    if (vers[0] != 3) {
        printf("Unsupported ID3 version: %d.%d\n", vers[0], vers[1]);
        fclose(fptr1);
        return;
    }
    int len = strlen(filename);
    if (len < 4 || strcmp(&filename[len - 4], ".mp3") != 0) {
        printf("Error: File extension is not .mp3\n");
        fclose(fptr1);
        return;
    }
    fseek(fptr1,0,SEEK_SET);
    char buffer[10];
    // Copy the header (first 10 bytes)
    fread(buffer, 1, 10, fptr1);
    fwrite(buffer, 1, 10, fptr2);

    printf("---------SELECTED EDIT OPTION---------\n\n");
    
    while (fread(frame_id, 1, 4, fptr1) == 4)
    {
        frame_id[4] = '\0';  // Null-terminate the frame ID

        fread(frame_size, 1, 4, fptr1);
        frame_data_size = GET_SIZE(frame_size);
        fread(frame_flags, 1, 2, fptr1);

        frame_data = (char *)malloc(frame_data_size);
        if (frame_data == NULL) {
            printf("Error allocating memory for frame data.\n");
            fclose(fptr1);
            fclose(fptr2);
            return;
        }

        fread(frame_data, 1, frame_data_size, fptr1);

        // Check if the current frame matches the tag we want to modify
        if (!modified && strcmp(argv[2], "-t") == 0 && strcmp(frame_id, "TIT2") == 0)
        {
            // Modify Title tag
            printf("----------CHANGED THE TITLE---------\n\n");
            printf("TITLE   :%s\n\n",new_value);
            fwrite(frame_id, 1, 4, fptr2);  // Write frame ID
            FIX_SIZE(frame_size, new_len);
            fwrite(frame_size, 1, 4, fptr2);  // Write new size
            fwrite(frame_flags, 1, 2, fptr2); // Write flags
            fputc(0x00, fptr2);
            fwrite(new_value, 1, new_len-1, fptr2); // Write new tag data
            modified = 1; // Mark as modified
            printf("-------TITLE CHANGED SUCCESSFULLY-------\n\n");

        }
        else if (!modified && strcmp(argv[2], "-a") == 0 && strcmp(frame_id, "TPE1") == 0)
        {
            // Modify Artist tag
            printf("----------CHANGED THE ARTIST---------\n\n");
            printf("ARTIST   :%s\n\n",new_value);
            fwrite(frame_id, 1, 4, fptr2);
            FIX_SIZE(frame_size, new_len);
            fwrite(frame_size, 1, 4, fptr2);
            fwrite(frame_flags, 1, 2, fptr2);
            fputc(0x00, fptr2);
            fwrite(new_value, 1, new_len-1, fptr2);
            modified = 1;
            printf("-------ARTIST CHANGED SUCCESSFULLY-------\n\n");
        }
        else if (!modified && strcmp(argv[2], "-A") == 0 && strcmp(frame_id, "TALB") == 0)
        {
            // Modify Album tag
            printf("----------CHANGED THE ALBUM---------\n\n");
            printf("ALBUM   :%s\n\n",new_value);
            fwrite(frame_id, 1, 4, fptr2);
            FIX_SIZE(frame_size, new_len);
            fwrite(frame_size, 1, 4, fptr2);
            fwrite(frame_flags, 1, 2, fptr2);
            fputc(0x00, fptr2);
            fwrite(new_value, 1, new_len-1, fptr2);
            modified = 1;
            printf("-------ALBUM CHANGED SUCCESSFULLY-------\n\n");

        }
        else if (!modified && strcmp(argv[2], "-y") == 0 && strcmp(frame_id, "TYER") == 0)
        {
            // Modify Year tag
            printf("----------CHANGED THE YEAR---------\n\n");
            printf("YEAR   :%s\n\n",new_value);
            fwrite(frame_id, 1, 4, fptr2);
            FIX_SIZE(frame_size, new_len);
            fwrite(frame_size, 1, 4, fptr2);
            fwrite(frame_flags, 1, 2, fptr2);
            fputc(0x00, fptr2);
            fwrite(new_value, 1, new_len-1, fptr2);
            modified = 1;
            printf("-------YEAR CHANGED SUCCESSFULLY--------\n\n");
        }
        else if (!modified && strcmp(argv[2], "-c") == 0 && strcmp(frame_id, "TCON") == 0)
        {
            // Modify Content tag
           printf("----------CHANGED THE CONTENT---------\n\n");
            printf("CONTENT   :%s\n\n",new_value);
            fwrite(frame_id, 1, 4, fptr2);
            FIX_SIZE(frame_size, new_len);
            fwrite(frame_size, 1, 4, fptr2);
            fwrite(frame_flags, 1, 2, fptr2);
            fputc(0x00, fptr2);
            fwrite(new_value, 1, new_len-1, fptr2);
            modified = 1;
            printf("-------CONTENT CHANGED SUCCESSFULLY--------\n\n");
        }
        else if (!modified && strcmp(argv[2], "-m") == 0 && strcmp(frame_id, "TCOM") == 0)
        {
            // Modify Comment tag
            printf("----------CHANGED THE COMPOSER---------\n\n");
            printf("COMPOSER   :%s\n\n",new_value);
            fwrite(frame_id, 1, 4, fptr2);
            FIX_SIZE(frame_size, new_len);
            fwrite(frame_size, 1, 4, fptr2);
            fwrite(frame_flags, 1, 2, fptr2);
            fputc(0x00, fptr2);
            fwrite(new_value, 1, new_len-1, fptr2);
            modified = 1;
            printf("-------COMPOSER CHANGED SUCCESSFULLY--------\n\n");
        }
        else
        {
            // Copy unmodified frames
            fwrite(frame_id, 1, 4, fptr2);
            fwrite(frame_size, 1, 4, fptr2);
            fwrite(frame_flags, 1, 2, fptr2);
            fwrite(frame_data, 1, frame_data_size, fptr2);
        }

        free(frame_data);
    }

    fclose(fptr1);
    fclose(fptr2);

    fptr1 = fopen(filename, "w");
    fptr2 = fopen("newfile.mp3", "r");
    copying(fptr1, fptr2);
    fclose(fptr1);
    fclose(fptr2);

    printf("MP3 tag edited successfully.\n");
}



int main(int argc, char *argv[])
{
    struct reader var;

    if (argc <= 2) {
        printf("ERROR: ./a.out : INVALID ARGUMENTS\n");
        printf("USAGE: \nTo View please pass like: ./a.out -v mp3filename\n");
        printf("To edit please pass like: ./a.out -e -t/-a/-A/-m/-y/-c mp3filename\n");
        printf("To help: ./a.out --help\n");
        return 1;
    }

    if (strcmp(argv[1], "-v") == 0) {
        if (argc != 3) {
            printf("ERROR: Missing filename for viewing.\n");
            return 1;
        }
        var.src_fname = argv[2];
        view_mp3_tag(var.src_fname);
    }
    else if (strcmp(argv[1], "-e") == 0)
    {
        if (argc != 5) {
            printf("ERROR: Missing filename for editing.\n");
            return 1;
        }
        var.src_fname = argv[4];
        edit_mp3_tag(argv, var.src_fname);
    }
    else if(strcmp(argv[1], "--help") == 0)
    {
        printf("1. -v to view mp3 file contents\n");
        printf("2. -e to edit mp3 file contents\n");
        printf("    -t Modifies a Title tag\n");
        printf("    -a Modifies an Artist tag\n");
        printf("    -A Modifies an Album tag\n");
        printf("    -y Modifies a Year tag\n");
        printf("    -m Modifies a Composer tag\n");
        printf("    -c Modifies a Content tag\n");
    }

    return 0;
}
