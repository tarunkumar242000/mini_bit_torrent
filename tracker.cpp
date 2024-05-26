#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<fstream>
#include<bits/stdc++.h>
#include<thread>
#include<sys/stat.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>

using namespace std;

#define CHUNK_SIZE 256
#define PORT 8100

pthread_t tid[50];


bool check(unsigned char *obuf,unsigned char *buf)
{
    int i;
    for(i=0;i<20;i++)
    {
        if(obuf[i]!=buf[i])
            return false;
    }
    return true;
}


void copy_content(unsigned char *buf,unsigned char *obuf)
{
    int i;
    for(int i=0;i<20;i++)
    {
        buf[i]=obuf[i];
    }
}


long sizeFile(string filename)                          //FILESIZE
{
    struct stat stat_buf;
    int rc=stat(filename.c_str(),&stat_buf);
    return rc==0 ? stat_buf.st_size : -1;
}


int do_sha1_file(char *name, unsigned char *out)
{
    FILE *f;
    unsigned char buf[8192];
    SHA_CTX sc;
    int err;

    f = fopen(name, "rb");
    if (f == NULL) {
        /* do something smart here: the file could not be opened */
        return -1;
    }

    SHA1_Init(&sc);
    for (;;) {
        size_t len;

        len = fread(buf, 1, sizeof buf, f);
        if (len == 0)
            break;
        SHA1_Update(&sc, buf, len);
    }

    err = ferror(f);
    fclose(f);
    if (err) {
        /* some I/O error was encountered; report the error */
        return -1;
    }

    SHA1_Final(out, &sc);
    return 0;
}

//GLOBAL PARAMETERS
int file_id=0;                          //FILE_ID
int group_id=0;                          //GROUP_ID
//int peer_id=0;                          //PEER_ID
unordered_set<int>group;                    //GROUP NUMBER


//FILE INFORMATION
class file
{
    public:
    
    int idd;
    string filename;
    unsigned char sha[20];
    unordered_set<int>peer_id;
    unordered_set<int>group_idd;
    long long size;
    long long chunks;

};


//PEER_INFORMATION
class peer
{
    public:

    int user_id;
    string password;

    int login;
    string IP;
    string PORT_NO;

    vector<int>group_leader;                    //PEER GROUP MEMBERS

    unordered_set<int>group_member;                    //PEER GROUPS

    unordered_set<file*>filename;
};



unordered_map<peer*,file*>seeder;                   //seeder
unordered_map<peer*,file*>leecher;                     //leecher
unordered_map<int,peer*>mp;                         //STORING PEER ID AND ITS CORRESPONDING POINTER AS GLOBAL
unordered_map<int,peer*>group_leader;               //STORING LEADER OF GROUP AND ITS CORRESPONDING POINTER
unordered_map<int,unordered_set<peer*>>group_members;      //STORING GROUPID AND MEMBERS IN IT
unordered_map<int,unordered_set<file*>>sharable_files;      //GROUP ID AND FILES SHARABLE IN IT
unordered_map<string,file*>sharable_file_name;  


// void* send_data(void* soocket)
// {
//     char* hello="kya hua";
//     int* new_socket=(int*)soocket;
//     send(*new_socket, hello, strlen(hello), 0);
//     printf("Hello message sent\n");

// }


// void* recieve_data(void* soket)
// {
//     int valread;
//     char buffer[1024]={0};
//     int* new1=(int*)soket;
//     while(1){
//     cout<<"new_socket"<<*new1<<endl;

//     valread = read(*new1, buffer, 1024);
//     cout<<valread<<endl;
//     printf("%s\n", buffer);
//     cout<<"new_socket"<<*new1<<endl;

//     bzero(buffer,1024);
//     }
//     cout<<"OUT"<<endl;
// }


void* client_handle(void* p_client_socket)
{
    int client_socket=*((int*)p_client_socket);
    free(p_client_socket);
   // cout<<client_socket<<endl;
    // pthread_t pd1;
    // pthread_t pd2;

    // pthread_create(&pd1,NULL,send_data,(void*)new_socket);
    // pthread_create(&pd2,NULL,recieve_data,(void*)new_socket);
    int valread;
    char buffer[1024]={0};
    int done=0;

   // cout<<"hello_client"<<endl;

    long long tot_chunk=0;
    string downloading;
    string IPP,PPORT;
    IPP="";
    PPORT="";
    int i=0;
    //cout<<"client_sockett"<<client_socket<<endl;
    valread=read(client_socket,buffer,1024);
    while(buffer[i]!=' ')
    {
        IPP+=buffer[i];
        i++;
    }

    //cout<<IPP<<endl;
   

    i++;
    while(i<valread)
    {
        PPORT+=buffer[i];
        i++;
    }
   // cout<<PPORT<<endl;

    bzero(buffer,1024);

    peer* new_peer=new peer;
    new_peer->PORT_NO=PPORT;
    new_peer->IP=IPP;
    
    file* new_file=new file;



    while(1){

   // cout<<5765765<<endl;
   
    int i=0;
    //cout<<client_socket<<endl;
    valread = recv(client_socket,buffer,1024,0);
  
    string temp1,temp2;
    while(buffer[i]!=' ')
         {
             temp1=buffer[i];
             temp2+=temp1;
             i++;
        }
    i++;

    // cout<<valread<<endl;
    send(client_socket,temp2.c_str(),temp2.length(),0);

    bzero(buffer,1024);
    valread = recv(client_socket,buffer,1024,0);
    i=0;
    temp1="";
    temp2="";
    while(buffer[i]!=' ')
         {
             temp1=buffer[i];
             temp2+=temp1;
             i++;
        }
    i++;
    //cout<<temp2<<endl;

    vector<string>vv;
    if(temp2=="create_user")
    {
        while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }
        // cout<<vv[0]<<endl;
        // cout<<vv[1]<<endl;
        new_peer->user_id=stoi(vv[0]);
        mp[stoi(vv[0])]=new_peer;
        new_peer->password=vv[1];
        vv.clear();
    }
    else if(temp2=="login")
    {
         while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }

        // cout<<mp[stoi(vv[0])]->password<<endl; 
        // cout<<vv[1]<<endl;
        if(mp.find(stoi(vv[0])) == mp.end())
        {
            cout<<"NO SUCH PEER EXIST"<<endl;
            close(client_socket);
        }
        else if(mp[stoi(vv[0])]->password != vv[1])
        {
            cout<<"Enter correct cerendtials"<<endl;
        }
        else
        {
            mp[stoi(vv[0])]->login=1;
        }
        vv.clear();
    }
    else if(temp2=="create_group")
    {
        if(new_peer->login)
        {while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }
        
        group.insert(stoi(vv[0]));
        vv.clear();
        }
        else
            cout<<"FIRST LOGIN THEN DO SOMETHING ELSE"<<endl;
    }
    else if(temp2=="join_group")
    {
        if(new_peer->login)
        {
            while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }

        if(group_leader.find(stoi(vv[0])) == group_leader.end())
        {
            group_leader[stoi(vv[0])]=new_peer;
            group_members[stoi(vv[0])].insert(new_peer);
            new_peer->group_member.insert(stoi(vv[0]));
        }
        else
        {
            group_members[stoi(vv[0])].insert(new_peer);
            new_peer->group_member.insert(stoi(vv[0]));
        }
        vv.clear();
        }
        else
        {
            cout<<"FIRST LOGIN"<<endl;
        }
    }
    else if(temp2=="leave_group")
    {
        while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }
        if(new_peer->group_member.find(stoi(vv[0])) == new_peer->group_member.end())
        {
            cout<<"PEER IS NOT PART OF THIS GROUP"<<endl;
        }
        else if( group_leader[stoi(vv[0])] == new_peer )
        {
            new_peer->group_member.erase(stoi(vv[0]));
            group_members[stoi(vv[0])].erase(new_peer);
            if(!group_members[stoi(vv[0])].empty())
            {
                auto it=group_members[stoi(vv[0])].begin();
                group_leader[stoi(vv[0])]=*it;
            }
            else
            {
                group_members.erase(stoi(vv[0]));
                group_leader.erase(stoi(vv[0]));
                group.erase(stoi(vv[0]));
            }
        }
        else
        {
            new_peer->group_member.erase(stoi(vv[0]));
            group_members[stoi(vv[0])].erase(new_peer);

            if(group_members[stoi(vv[0])].empty())
            {
                group_members.erase(stoi(vv[0]));
                group_leader.erase(stoi(vv[0]));
                group.erase(stoi(vv[0]));
            }
           
        }
        vv.clear();
    }
    else if(temp2=="file")
    {
        while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }
        cout<<sharable_file_name[vv[0]]->filename<<endl;
        cout<<sharable_file_name[vv[0]]->size<<endl;
        cout<<sharable_file_name[vv[0]]->chunks<<endl;
        
        for(int i=0;i<20;i++)
        {
            printf("%x",sharable_file_name[vv[0]]->sha[i]);
        }
        vv.clear();
    }
    // else if(temp2=="list_requests")
    // {
        
    // }
    // else if(temp2=="accept_request")
    // {
        
    // }
    else if(temp2=="list_groups")
    {
        if(group.size()==0)
        {
            cout<<"no group exists";
        }
        auto it=group.begin();
        for(;it!=group.end();it++)
        {
            cout<<*it<<endl;
        }
    }
    else if(temp2=="list_files")
    {
        while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }
        auto it=sharable_files[stoi(vv[0])].begin();
        for(;it!=sharable_files[stoi(vv[0])].end();it++)
        {
            cout<<(*it)->filename<<endl;
        }
        vv.clear();
    }
    else if(temp2=="upload_file")
    {
        //cout<<"upload_file"<<endl;
        while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }
        unsigned char buffer1[20];

       // cout<<vv[0]<<endl;

        do_sha1_file((char*)vv[0].c_str(),buffer1);
        // for(int i=0;i<20;i++)
        // {
        //     printf("%x",buffer1[i]);
        // }
        
        new_peer->filename.insert(new_file);                        //adding file info to peer
        new_file->filename=vv[0];
        copy_content(new_file->sha,buffer1);                         //copy content to file sha buffer
        
        
        new_file->peer_id.insert(new_peer->user_id);                //storing file is part of which peer
        new_file->group_idd.insert(stoi(vv[1]));

         long size=sizeFile(vv[0]);

       // cout<<size<<endl;
        //cout<<"size"<<endl;

        new_file->size=size;
        if(size/CHUNK_SIZE)
            new_file->chunks=size/CHUNK_SIZE + 1;
        else
            new_file->chunks=size/CHUNK_SIZE;

       // cout<<new_file->chunks<<endl;

        sharable_files[stoi(vv[1])].insert(new_file);     //adding group id and sharable files inside it

        sharable_file_name[vv[0]]=new_file;              //adding file name and its pointer
        vv.clear();
    }
    else if(temp2=="download_file")
    {
        while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }
         downloading=vv[1];

        file* point=sharable_file_name[vv[1]];
        if(group_members[stoi(vv[0])].find(new_peer) == group_members[stoi(vv[0])].end())    //CHECKING PART OF GROUP
        {
            cout<<"NOT PART OF GROUP SO CANT SHARE FILES"<<endl;
        }
        else if(sharable_files[stoi(vv[0])].find(point) == sharable_files[stoi(vv[0])].end())   //WHETHER FILE IS SHARREABLE 
        {
            cout<<"File is not  shareable "<<endl;
        }
        else 
        {
            char b[1024];
            string str;
            auto it=sharable_file_name[vv[1]]->peer_id.begin();
            for(;it!=sharable_file_name[vv[1]]->peer_id.end();it++)
            {
                str+=mp[*it]->IP;
                str+=" ";
                str+=mp[*it]->PORT_NO;
                str+=" ";
            }
          //  cout<<str<<endl;
            send(client_socket,str.c_str(),str.length(),0);
            //recv(client_socket,b,1024,0);
        }

    }
    else if(temp2=="logout")
    {
        continue;
    }
    // else if(temp2=="show_downloads")
    // {
        
    // }
    // else if(temp2=="stop_share")
    // {
        
    // }
    else if(temp2 == "upload")
    {
        cout<<"hello"<<endl;
        while(i<valread)
        {    
            temp2="";
            while((buffer[i]!=' ') && (i<valread))
            {
                temp1=buffer[i];
                temp2+=temp1;
                i++;
            }
            i++;
            vv.push_back(temp2);
        }


        file* pinter;
        
        file* fiile=new file;
        pinter=fiile;
        fiile->filename=vv[0];

        auto it=new_peer->group_member.begin();
        for(;it!=new_peer->group_member.end();it++)
        {
            sharable_files[*it].insert(fiile);
        }

        sharable_file_name[vv[0]]=fiile;
        
        //tot_chunk+=stoi(vv[0]);
        //pinter->chunks=tot_chunk;
        // if(tot_chunk==sharable_file_name[downloading]->chunks)
        // {
            unsigned char buffer[20];
            do_sha1_file((char*)vv[0].c_str(),buffer);
            copy_content(pinter->sha,buffer);
            if(check(buffer,sharable_file_name[downloading]->sha))
            {
                cout<<"file reicieved correctly";
            }
            else
            {
                cout<<"FILE is corrupted";
            }
        //}
    }
    else
    {
        cout<<"Wrong Commmand"<<endl;
        cout<<"Enter correct command"<<endl;
    }

    bzero(buffer,1024);
    }
   
    return NULL;
}

int main(int argc,char* argv[])
{

    if(argc<3)
    {
        cout<<"Input correct values"<<endl;
        return 0;
    }

    vector<string>tracker_info;

    for(int i=1;i<3;i++)
    {
        tracker_info.push_back(argv[i]);
    }

    string tracker_file=tracker_info[0];

    char tracker_filee[tracker_file.length()+1];

    strcpy(tracker_filee,tracker_file.c_str());

    ifstream fin;
    fin.open(tracker_filee,ifstream::in);

    string tracker_ip,tracker_port;

    fin>>tracker_ip>>tracker_port;

    //cout<<tracker_ip<<endl;
    //cout<<tracker_port<<endl;

    int server_fd,new_socket;
    struct sockaddr_in address;
    char buffer[1024] = { 0 };
    
     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {              
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
   
    int opt=1;
    if (setsockopt(server_fd, SOL_SOCKET,                                   
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
 

    address.sin_family = AF_INET;                                               
    address.sin_addr.s_addr = inet_addr(tracker_ip.c_str());
    address.sin_port = htons(stoi(tracker_port));

    int addrlen = sizeof(address);


 if (bind(server_fd, (struct sockaddr*)&address,                                 
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int ii=0;
    while(1)
    {
        new_socket= accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen);
        //cout<<new_socket<<endl;
        pthread_t p1;
        int *pclient=(int*)malloc(sizeof(int));
        *pclient=new_socket;
        pthread_create(&p1,NULL,client_handle,pclient);
        //pthread_join(p1,NULL);
    }



    return 0;
}