
#include <CacheFuction.h>
#include <string.h>
const char *key0 = "valid";
const char *key1 = "size";
struct HPCsystem
{
    char *PFS;
    char *BB0;
    char *BB1;
    char *json_file; 
    double threshold;
    /* data */
};
struct HPCsystem config = {
    "/home/ubutnu/hardDisk/PFS/nocompress/",
    "/home/ubutnu/hardDisk/BB/nocompress/",
    "/home/ubutnu/hardDisk/BB/nocompress/",
    "/home/ubutnu/Application/Neural-network/Cache/file_cache_nocompress.json",
    1024*1024*1024*2.5
};

int Prefetch(const char *filename) {
    struct timeval start;
    struct timeval end;
    gettimeofday(&start,NULL);
    char file_PFS[100];
    char file_BB[100];

    sprintf(file_PFS,"%s%s",config.PFS,filename);
    sprintf(file_BB,"%s%s",config.BB1,filename);

    char command[220];
    sprintf(command, "cp -r %s %s", file_PFS, file_BB);
    
    // 调用system函数执行cp命令
    int status = system(command);
    
    if (status == 0) {
        fprintf(stderr,"Prefetch file(%s) Successfully.\n",filename);
    } else {
        fprintf(stderr,"Error Command: %s\n",command);
        return 0;
    }
    gettimeofday(&end,NULL);
    float time_use=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
    fprintf(stderr,"Prefetch time_use is %.10f us\n",time_use);
    return 0;

}

char *demotion() {
    // Implementation of demotion_data function
    char command[1024];

    struct timeval start;
    struct timeval end;
    gettimeofday(&start,NULL);
    char *PFS = config.PFS;
    char *BurstBuffer = config.BB1;
    // Construct command string
    sprintf(command, "ls -cr -I 'bk_*' %s", BurstBuffer);

    // Execute command and capture output
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        return NULL;
    }
    char *coldestfile = (char*)malloc(100);
    fscanf(fp,"%s",coldestfile);
    sprintf(command, "cp -r %s%s %s\nmv %s%s %sbk_%s",BurstBuffer, coldestfile, PFS, BurstBuffer, coldestfile, BurstBuffer,coldestfile);
    
    
    int status = system(command);
    if (status == 0) {
        fprintf(stderr,"Demote file(%s) successfully.\n",coldestfile);
    } else {
        fprintf(stderr,"Error Command: %s\n",command);
        return NULL;
    }
    gettimeofday(&end,NULL);
    float time_use=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
    fprintf(stderr,"Demoting time_use is %.10f us\n",time_use);
    // Close file pointer
    pclose(fp);

    return coldestfile;
}

int Read_file_cache(const char *filepath) {
    char *json_file = config.json_file;
    char *ret;
    const char *filename; 

    ret = strrchr(filepath, '/');
    if(ret != NULL)
        filename = ret+1;
    else filename = filepath;

    double threshold= config.threshold;
    FILE *F = fopen(json_file, "r");
    if (F == NULL) {
        printf("Error opening josn file!\n");
        return 0;
    }
    
    fseek(F, 0, SEEK_END);
    int length = ftell(F);
    fseek(F, 0, SEEK_SET);

    char *buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, F);
    fclose(F);

    cJSON *root = NULL;
    root = cJSON_Parse(buffer);
    free(buffer);

    cJSON *json_data1 = cJSON_GetObjectItemCaseSensitive(root,filename);
    cJSON *vld = cJSON_GetObjectItemCaseSensitive(json_data1,key0);

    if(vld->valueint == 1){
        cJSON *json_data = cJSON_GetObjectItemCaseSensitive(root,"ALL");
        cJSON *all_size = cJSON_GetObjectItemCaseSensitive(json_data,key1);
        cJSON *size = cJSON_GetObjectItemCaseSensitive(json_data1,key1);
        
        double next_size = all_size->valuedouble + size->valuedouble;
        cJSON *temp = NULL, *temp_size = NULL;
        while ( next_size > config.threshold) {
            char *file_d = demotion();
            temp = cJSON_GetObjectItemCaseSensitive(root,file_d);
            temp_size = cJSON_GetObjectItemCaseSensitive(temp,key1);
            next_size -= temp_size->valuedouble; 

            cJSON *new_item = cJSON_CreateNumber(1);       
            cJSON_ReplaceItemInObject(temp,key0,new_item);
        }
        Prefetch(filename);
        cJSON_SetNumberValue(vld, 0);
        cJSON_SetNumberValue(all_size,next_size);

        F = fopen(json_file, "w");
        if (F == NULL) {
            fprintf(stderr,"Error writing json file!\n");
            return 1;
        }

        char *json_string = cJSON_Print(root);
        fputs(json_string, F);
        fclose(F);
    }
    else {
        fprintf(stderr,"Direct read file(%s)!\n",filename);
    }
    return 0;
}


int Write_file_cache(const char *filepath) {
    char *json_file = config.json_file;
    char *ret;
    const char *filename; 

    ret = strrchr(filepath, '/');
    if(ret != NULL)
        filename = ret+1;
    else filename = filepath;

    double threshold= config.threshold;

    FILE *F = fopen(json_file, "r");
    if (F == NULL) {
        printf("Error opening josn file!\n");
        return 0;
    }
    fseek(F, 0, SEEK_END);
    int length = ftell(F);
    fseek(F, 0, SEEK_SET);

    char *buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, F);
    fclose(F);
    
    cJSON *root = NULL;
    root = cJSON_Parse(buffer);
    free(buffer);

    cJSON *json_data = cJSON_GetObjectItemCaseSensitive(root,"ALL");
    cJSON *all_size = cJSON_GetObjectItemCaseSensitive(json_data,key1);

    cJSON *json_data1 = cJSON_GetObjectItemCaseSensitive(root,filename);
    cJSON *vald = cJSON_GetObjectItemCaseSensitive(json_data1,key0);

    double next_size =  all_size->valuedouble;
    if(vald->valueint != 0) {
        cJSON *wrt_size = cJSON_GetObjectItemCaseSensitive(json_data1,key1);
        cJSON_SetNumberValue(vald, 0);
        next_size += wrt_size->valuedouble;
    }
    if ( next_size <= threshold) {
        fprintf(stderr,"Direct write file(%s)!\n",filename);
    }

    // double next_size = 0;
    cJSON *temp = NULL, *temp_size = NULL;
    while( next_size > threshold) {
        //printf("size: %lf\n",next_size);
        char *file_d = demotion();
        temp = cJSON_GetObjectItemCaseSensitive(root,file_d);
        temp_size = cJSON_GetObjectItemCaseSensitive(temp,key1);
        next_size -= temp_size->valuedouble; 

        cJSON *new_item = cJSON_CreateNumber(1);       
        cJSON_ReplaceItemInObject(temp,key0,new_item);
    }
    
    all_size->valuedouble = next_size;
    cJSON_SetNumberValue(all_size, next_size);
    
    F = fopen(json_file, "w");
    if (F == NULL) {
        fprintf(stderr,"Error writing joson file!\n");
        return 1;
    }
    
    char *json_string = cJSON_Print(root);
    fputs(json_string, F);
    fclose(F);
    char *BBcsd = config.BB0;
    char *BB=config.BB1;
    if(BB!=BBcsd) {
        char command[200];
        sprintf(command, "cp %sbk_%s %s%s",BB, filename, BB, filename);
        int status = system(command);
        if (status == 0) {
            fprintf(stderr,"write BBcsd successfully.\n");
        } else {
            fprintf(stderr,"Error Command: %s\n",command);
            return 0;
        }
    }
    return 0;
}

int show_function(const char *filename){
    fprintf(stderr,"hook successful! filename: %s\n",filename);
}