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

string bhejna_ip;
unordered_map<long long,set<long long>>mp;
int aya=0;
int uploaad=0;
string download_file;
string destination_path;
string fname;
int cclient_sockeet;

string actual_name()
{
    string act;
    int n=download_file.length();
    int i=n-1;
    while(download_file[i]!='.')
    {
        i--;
    }
    i--;
    while(download_file[i]!='/')
    {
        act+=download_file[i];
        i--;
    }
    reverse(act.begin(),act.end());
    return act;
}

string file_extension()
{
    string ext;
    int n=download_file.length();
    int i=n-1;
    while(download_file[i]!='.')
    {
        ext+=download_file[i];
        i--;
    }
    reverse(ext.begin(),ext.end());
    return ext;
}

class file
{
    public:

    string filename;
    unordered_set<int>chunks;
    unordered_map<int,int>chunk_freq;
    int upload;
    int Chunks;
    long filesize;
};

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



// class file
// {
//     public:

//     string filename;
//     unordered_set<int>chunks;
//     unordered_map<int,int>chunk_freq;
//     int upload;
//     int Chunks;
// };


unordered_map<string,file*>filee;  //store filename and its pointer
unordered_map<string,file*>filee_req;
string trac_ip,track_port;
string IP1,port1;



vector<pair<string,string>>v0;

// void* recieve_data1(void* suket)
// {
//     int client_socket=*((int*)sucket);

//     int valread;
//     char buffer[1024];
    
//     valread=read(client_socket,buffer,1024);

//      vector<string>vv;
//     string temp1,temp2;
//     while(i<valread)
//     {    
//         temp2="";
//         while((buffer[i]!=' ') && (i<valread))
//         {
//             temp1=buffer[i];
//             temp2+=temp1;
//             i++;
//         }
//         i++;
//         vv.push_back(temp2);
//     }


// }

void* send_data1(void* nusoket)
{
    int client_socket=*((int*)nusoket);
    char buffer1[1024];
    int valread=recv(client_socket,buffer1,1024,0);
     fname="";
    int i=0;
    while(i<valread)
    {    
        while((buffer1[i]!=' ') && (i<valread))
        {
            fname+=buffer1[i];
            i++;
        }
        i++;
    }

    // vector<int>tott_chunky;
    // string tot_chunk;
    // auto it=filee[fname]->chunks.begin();
    // for(;it!=filee[fname]->chunks.end();it++)
    // {
    //     tot_chunk+=to_string(*it)+" ";
    //     tott_chunky.push_back(*it);
    // }
    // send(client_socket,tot_chunk.c_str(),tot_chunk.length(),0);
    off_t offset=0;
    int fd = open(fname.c_str(), O_RDONLY);
        char buffer[256];
        while(1)
        {
                int sent_bytes=pread(fd,buffer,256,offset);



                if(sent_bytes<=0)
                {
                        int senddd=send(client_socket,buffer,sent_bytes,0);
                        break;
                }


                int temp=sent_bytes;
               while(sent_bytes>0)
                {
                        int sendd=send(client_socket,buffer,sent_bytes,0);
                        sent_bytes-=sendd;
               }


                offset+=temp;
                bzero(buffer,256);
        }
        close(fd);
       pthread_exit(NULL);
}


void* client_handler(void* a)                           
{
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
    address.sin_addr.s_addr = inet_addr((IP1.c_str()));
    address.sin_port = htons(stoi(port1));

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


    while(1)
    {

        if ((new_socket
            = accept(server_fd, (struct sockaddr*)&address,
                    (socklen_t*)&addrlen))
            < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
    }

    pthread_t p;
    pthread_t p1;
    pthread_create(&p,NULL,send_data1,(void*)&new_socket);
    pthread_join(p,NULL);
    close(new_socket);
    //pthread_create(&p1,NULL,recieve_data1,(void*)&new_socket);
    }
}



void* recieve_data(void* sockettt)
{
    //cout<<"receive_data_entry"<<endl;
    int client_socket=*((int*)sockettt);
    //cout<<"client_socket"<<client_socket<<endl;
    char buffer[1024];
    int valread;
    string abcc="random";
    vector<string>vv;
    string temp1,temp2;

   valread=bhejna_ip.length();
   //cout<<valread<<endl;
    //cout<<bhejna_ip<<endl;
    int i=0;
    while(i<valread)
    { 
            //cout<<"reading"<<endl;   
            temp2="";
            while((bhejna_ip[i]!=' ') && (i<valread))
            {
                temp1=bhejna_ip[i];
                temp2+=temp1;
                i++;
            }
            i++;
            //cout<<temp2<<endl;
            vv.push_back(temp2);
    }

    //cout<<"segment ayay kya"<<endl;

    vector<pair<string,string>>v1;

    int n=vv.size();
    int ii=0;
    while(n)
    {
        string hh=vv[ii];
        //cout<<hh<<endl;
        ii++;
        string jj=vv[ii];
        //cout<<jj<<endl;
        ii++;
        n=n-2;
        v1.push_back(make_pair(hh,jj));
    }
    //cout<<"idhar ay a kya segmentation fault"<<endl;
    
    int a=1;
    // pthread_t pdd;
    // pthread_create(&pdd,NULL,peer_handler,(void*)&a);
    n=v1.size();

    for(int i=0;i<n;i++)                                            //GETTINGS CHUNKS DETAILS
    {
        int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {    
        printf("\n Socket creation error \n");
        exit(1);
    }
    
    // //cout<<v1[i].second<<endl;
    // //cout<<v1[i].second.length()<<endl;
    // //cout<<v1[i].first<<endl;
    // //cout<<v1[i].first.length()<<endl;


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(stoi(v1[i].second));

    if (inet_pton(AF_INET,v1[i].first.c_str() , &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        exit(1);
    }
    //cout<<11<<endl;
    if ((client_fd
         = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }
    
     send(sock,download_file.c_str(),download_file.length(),0);

    char buffer1[256];

    string extension=file_extension();
    string actual=actual_name();


    //cout<<"actual"<<actual<<endl;
    string abc=destination_path + "/" + actual + "." + extension;

    //cout<<"abc"<<abc<<endl;


    int received_file=open(abc.c_str(),O_WRONLY | O_APPEND | O_CREAT, 0644);
    //cout<<"kyaa h be"<<endl;
    //cout<<143<<endl;
    
    off_t offset=0;

        while(1)
        {
                int len=recv(sock,buffer1,256,0);
    
                if(len<=0)
                        break;
                long long temp=len;
               while(temp>0)
               {
                        long long written=pwrite(received_file,buffer1,temp,offset);
                        temp-=written;
               }
                offset+=len;
                len=0;
                bzero(buffer1,256);
        }

        vv.clear();
        close(received_file);
        bzero(buffer1,256);
        string abbb;
        abbb+="upload";
        string axt;
        axt+=abbb +" ";
        axt+=abc;


      
        send(cclient_sockeet,axt.c_str(),axt.length(),0);


    }


    
}




int main(int argc,char* argv[])
{

    if(argc<3)
    {
        cout<<"Input correct values"<<endl;
        return 0;
    }
    vector<string>peer_info;

    for(int i=1;i<3;i++)
    {
        peer_info.push_back(argv[i]);
    }

    string peer_infoo=peer_info[0];

   // cout<<peer_infoo<<endl;
    int i=0;
    string IP="";                                            //PEER IP
    while(peer_infoo[i]!=':')
    {
        IP+=peer_infoo[i++];
    }
    
    IP1=IP;
    //cout<<IP1<<endl;
    i++;
    int n=peer_infoo.length();
    //cout<<n<<endl;
    string port="";                                            //PEER PORT
    while(i<n)
    {
        port+=peer_infoo[i];
        i++;
    }
    

    port1=port;
   // cout<<port1<<endl;

    char tracker_file[peer_info[1].length()+1];

   // cout<<1<<endl;
    strcpy(tracker_file,peer_info[1].c_str());
    //cout<<2<<endl;

    ifstream fin;
    //cout<<3<<endl;

    fin.open(tracker_file,ifstream::in);
    //cout<<4<<endl;

    string tracker_ip,tracker_port;                                 //TRACKER IP AND PORT
    fin>>tracker_ip>>tracker_port;

    trac_ip=tracker_ip;
    track_port=tracker_port;

    int a=1;

   // cout<<"hello"<<endl;
   // cout<<track_port<<endl;
   // cout<<trac_ip<<endl;

    // pthread_t server;

    // pthread_create(&server,NULL,client_handler,(void*)&a);

    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {    
        printf("\n Socket creation error \n");
        exit(1);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(stoi(track_port));

    if (inet_pton(AF_INET,trac_ip.c_str() , &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        exit(1);
    }
 
    if ((client_fd
         = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }

    pthread_t pd2;
    //cout<<"sock"<<sock<<endl;
    pthread_create(&pd2,NULL,client_handler,(void*)&sock);
    string sizee;
    cclient_sockeet=sock;
    string s="";
    s+=IP1+" "+port1;
    //cout<<s<<endl;
    send(sock,s.c_str(),s.length(),0);
    string line,temp1,temp2;
    vector<string>imp;
    char buffer[1024];
    while(getline(cin,line))
    {
        //cout<<2233<<endl;
        // sizee=to_string(line.length());
        // sizee+=" ";
        // int c=send(sock,sizee.c_str(),sizee.length(),0);

       // cout<<"c"<<c<<endl;
        // recv(sock,buffer,1024,0);
        // int val=send(sock,line.c_str(),(int)line.length(),0);
       // cout<<val<<endl;
        int i=0;
        while(i<line.length())
        {    
            temp2="";
            while((line[i]!=' ') && (i<line.length()))
            {
                temp1=line[i];
                temp2+=temp1;
                i++;
            }
            i++;
            imp.push_back(temp2);
        }

        if(imp[0]=="upload_file")
           {

            int chunks;
            uploaad=1;
            file* new_file=new file;
            long  size=0;
            size=sizeFile(imp[1]);
            
             if(size/CHUNK_SIZE)
                 chunks=size/CHUNK_SIZE + 1;
            else
                 chunks=size/CHUNK_SIZE;

            new_file->filesize=size;

            new_file->upload=1;
            new_file->Chunks=chunks;
            filee[imp[1]]=new_file;
            for(int i=0;i<chunks;i++)
            {
                new_file->chunks.insert(i);
            }
            sizee=to_string(line.length());
            sizee+=" ";
            int c=send(sock,sizee.c_str(),sizee.length(),0);
            recv(sock,buffer,1024,0);
            send(sock,line.c_str(),line.length(),0);
            imp.clear();
           }
        else if(imp[0]=="download_file")
        {
            download_file=imp[2];
            destination_path=imp[3];

            sizee=to_string(line.length());
            sizee+=" ";
            int c=send(sock,sizee.c_str(),sizee.length(),0);
            recv(sock,buffer,1024,0);
            send(sock,line.c_str(),line.length(),0);
            bzero(buffer,1024);
            int vf=recv(sock,buffer,1024,0);                       //download from these ports

            for(int i=0;i<vf;i++)
            {
                bhejna_ip+=buffer[i];
            }

            pthread_t p1;
            int acr;
            pthread_create(&p1,NULL,recieve_data,(void*)&acr);
            pthread_join(p1,NULL);
            imp.clear();
        }
        else
        {

            sizee=to_string(line.length());
            sizee+=" ";
            int c=send(sock,sizee.c_str(),sizee.length(),0);
            recv(sock,buffer,1024,0);
            send(sock,line.c_str(),line.length(),0);
           
             imp.clear();
        }
    }

    
   
    
    return 0;
}