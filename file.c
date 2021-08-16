#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "file.h"

void changeContent(char* org, char* after){
    memset(org, 0, strlen(org));
    free(org);
    org = calloc(strlen(after)+1, sizeof(char));
    strcpy(org, after);
}

void copyPointer(fileNode** p1, fileNode** p2){
    *p1 = *p2;
}
fileTree* newFileTree() {
    fileTree *ft = calloc(1, sizeof(fileTree));
    memset(ft, 0, sizeof(fileTree));
    return ft;
}

fileNode* newFileNode(int type, int exist, char* name, char* MD5, char* path){
    fileNode* fn = calloc(1, sizeof(fileNode));
    fn->type=type;
    fn->exist = exist;
    fn->name = calloc(strlen(name)+1, sizeof(char));
    fn->MD5 = calloc(strlen(MD5)+1, sizeof(char));
    fn->path = calloc(strlen(path)+1, sizeof(char));
    strcpy(fn->name, name);
    strcpy(fn->MD5, MD5);
    strcpy(fn->path, path);
    return fn;
}

void freeFileNode(fileNode* fn){
    memset(fn->MD5, '\0', strlen(fn->MD5));
    free(fn->MD5);
    memset(fn->name, '\0', strlen(fn->name));
    free(fn->name);
    memset(fn->path, '\0', strlen(fn->path));
    free(fn->path);
    memset(fn, '\0', sizeof(fileNode));
    free(fn);
}

void printFileNode(fileNode* fn, int depth){
    printf("%*sType:%d Exist:%d Name:%s MD5:%s Path:%s\n\n", depth, " ",fn->type, fn->exist, fn->name, fn->MD5, fn->path);
}

void printFileTree(fileNode * fn, int depth) {
    for (fileNode * curr = fn; curr != NULL; curr = curr->next) {
        if (curr->type == 0) {
            printf("DirName:%s fullPath:%s level:%d\n",curr->name, curr->path, depth/4);
            printFileTree(curr->item, depth+4);
        } else {
            printFileNode(curr, depth);
        }
    }
}

int countToken(char *path) {
    char pathCp[strlen(path) + 1];
    strcpy(pathCp, path);
    int count = 0;
    for (char *token = strtok(pathCp, "/"); token != NULL; token = strtok(NULL, "/")) {
        count++;
    }
    return count;
}

//return null if the dir doesn't exist
void appendNode(fileNode** fn, fileNode* item) {
    if(*fn==NULL){
        *fn = item;
        return;
    }
    fileNode * curr = *fn;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = item;
    item->pre = curr;
}

fileNode* findNode(fileNode* fn, char* name, int type){
    if(fn == NULL) return NULL;
    fileNode * curr = fn;
    while(curr != NULL){
        if(strcmp(curr->name, name) == 0 && curr->type == type){
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}



static int defineType(int i, int count, int type){
    if(i<count-1 || type==0) return 0;
    else return 1;
}

void addFileNode(fileTree * ft, int type, char* MD5, char* path){
    char pathCp[strlen(path)+1];
    strcpy(pathCp, path);

    int count = countToken(path);
    int i = 0;

    char fullDirPath[260] = "";

    fileNode** curr;
    for(char *token = strtok(pathCp, "/"); token != NULL; token = strtok(NULL, "/")){
        //if the token is root
        //get the path for dir
        strcat(fullDirPath, token);
        strcat(fullDirPath,"/");
        //printf("%s\n",dirPath);
        if(strcmp(token, ".") == 0) {
            curr = &ft->head;
        }
        //try to find the token
        //if the token exist, cd into it
        else if(findNode(*curr, token, defineType(i,count,type)) != NULL){
            //if it is a dir, then cd
            if(i<count-1) curr = &findNode(*curr,token, 0)->item;
        }
        //if the token doesn't exist, creat and append it to the list
        else{
            //if it is not last token, add a dir and cd into it
            if(i<count-1){
                fullDirPath[strlen(fullDirPath)-1] = 0;
                fileNode* item = newFileNode(0, 1, token, "null", fullDirPath);
                strcat(fullDirPath,"/");
                appendNode(curr, item);
                curr = &findNode(*curr, token, 0)->item;
            }
            //if it is last token, creat a node with input type and add it
            else if(i == count-1){
                fileNode* item;
                if(type == 0) item = newFileNode(type, 1, token, "null", path);
                if(type == 1) item = newFileNode(type, 1, token, MD5, path);
                appendNode(curr, item);
            }
        }
        i++;
    }
}

//delete target fileNode and return new head
fileNode* deleteFileNode(fileNode* head, fileNode* target){
    fileNode* curr;
    //if target is the first one on the list
    if(head == target){
        curr = head->next;
        freeFileNode(head);
        return curr;
    }
    //if target is in the middle
    else if(target->next != NULL){
        curr = target;
        target->pre->next = target->next;
        target->next->pre = target->pre;
        freeFileNode(curr);
        return head;
    }
    else{
        target->pre->next = NULL;
        freeFileNode(target);
        return head;
    }
}

//return the address of fileNode, all mark the whole path exist
fileNode* traceNode(fileTree* ft, char* path, int type){
    char pathCp[strlen(path)+1];
    strcpy(pathCp, path);

    int count = countToken(path);
    int i = 0;

    //printf("%s\n", pathCp);
    fileNode* curr;
    for(char *token = strtok(pathCp, "/"); token != NULL; token = strtok(NULL, "/")){
        //printf("%d:%s\n", i, token);

        if(strcmp(token, ".")==0){
            curr = ft->head;
            if (curr == NULL) return NULL;
            curr->exist = 1;
        }
        else if(i < count-1){
            //printf("keep tracing\n");
            if(findNode(curr, token, 0) == NULL){
                return NULL;
            }
            else {
                //printf("cd\n");
                curr = findNode(curr, token, 0);
                curr->exist = 1;
                curr = curr->item;
            }
        }
        else if(i == count-1){
            if(findNode(curr, token, type)==NULL){
                return NULL;
            }
            else{
                curr = findNode(curr, token, type);
                curr->exist = 1;
                return curr;
            }
        }
        i++;
    }
    return NULL;
}

void updateFile(fileTree* ft, int type, char* MD5, char* path){
    //search fileNode and return fileNode
    fileNode* fn = traceNode(ft, path,type);
    //if file exist
    if(fn != NULL){
        //compare MD5
        if(strcmp(MD5, fn->MD5) != 0){
            changeContent(fn->MD5, MD5);
            printf("update MD5: ");
            printFileNode(traceNode(ft, path, type), 0);
        }
    }
    //if file doesn't exist
    else{
        //add file into
        addFileNode(ft, type, MD5, path);
        printf("add node: ");
        printFileNode(traceNode(ft, path, type), 0);
    }
}

//mark all file exist to zero
void clearAllExist(fileNode* fn){
    for(fileNode* curr = fn; curr != NULL; curr = curr->next){
        //mark exist as 0
        curr->exist = 0;
        //if curr is a dir then recursive call on their item
        if(curr->type == 0){
            clearAllExist(curr->item);
        }
    }
}

//recursive all file not exist in the dir
fileNode* deleteAllFileNotExist(fileNode* fn){
    fileNode* newHead = fn;
    fileNode* curr = fn;
    while(curr != NULL){
        if(curr->type == 0){
            curr->item = deleteAllFileNotExist(curr->item);
        }
        if(curr->exist == 0){
            //printf("intend to delete:");
            //printFileNode(curr, 0);
            //if curr is the head
            if(curr == newHead){
                //printf("curr == newHead\n");
                newHead = deleteFileNode(newHead, curr);
                curr = newHead;
            }
            //if curr is not the head
            else{
                fileNode* t = curr->next;
                newHead = deleteFileNode(newHead, curr);
                curr = t;
            }
        }
        else if(curr->exist == 1){
            curr = curr->next;
        }
    }
    return newHead;
}

static fileNode* testNewFileNode(){
    fileNode* a = newFileNode(1, 1,"file 0", "0", "./path0");
    return a;
}



int main() {
    fileTree *ft = newFileTree();
    addFileNode(ft, 0,  "null", "./dir");
    addFileNode(ft, 1,  "3", "./dir/file3");
    addFileNode(ft, 1,  "4", "./dir/file4");
    addFileNode(ft, 0,  "null", "./dir/dir1");
    addFileNode(ft, 1,  "5", "./dir/dir1/dir2/dir3/file5");
    addFileNode(ft, 1,  "3.1", "./dir/dir1/dir2/file3.1");
    addFileNode(ft, 1,  "3.2", "./dir/dir1/dir2/file3.2");
    addFileNode(ft, 0,  "null", "./dir/dir2");
    printf("print tree now:\n");
    //fileNode* fn = traceNode(ft, "./dir/dir1/dir2/file3.1", 1);
    //printFileNode(fn, 0);
    clearAllExist(ft->head);
    //updateFile(ft, 1, "3.1.1", "./dir/dir1/dir2/file3.1");
    //updateFile(ft, 1,  "3.2", "./dir/dir1/dir2/file3.2");
    //updateFile(ft, 0,  "null", "./dir/dir2");
    //updateFile(ft, 1,  "3", "./dir/file3");
    //deleteAllFileNotExist(ft->head);
    //freeFileNode(fn);
    printFileTree(ft->head, 0);
    //ft->head->item = deleteFileNode(ft->head->item, ft->head->item);
    //printFileNode(ft->head->item->next, 0);
    //add all file

    //while
        //mark all file doesn't exist
        //update each file(find file edit and (file doesn't exist and add up))
        //delete file doesn't exist (find what file doesn't exist anymore)
    return 0;
}
