#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <map>
#include <sstream>
#include <vector>


using namespace std;

int main()
{
    // Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1)
    {
        cerr << "Can't create a socket!";
        return -1;
    }
    // Bind the socket to a IP / port
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1)
    {
        cerr << "Can't bind to IP/port" << endl;
        return -2;
    } 
    // Mark the socket for listening in 
    if (listen(listening, SOMAXCONN) == -1)
    {
        cerr << "Can't listen!";
        return -3;
    }
    // Accept a call 
    sockaddr_in client;
    socklen_t clientSize;
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clientSocket = accept(listening,
                              (sockaddr*)&client,
                              &clientSize);
    if (clientSocket == -1) 
    {
        cerr << "Problem with client connecting!";
        return -4;
    
    }
    
    // Close the listening socket 
    close(listening);

    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((sockaddr*)&client,
                             sizeof(client),
                             host,
                             NI_MAXHOST,
                             svc,
                             NI_MAXSERV,
                             0);

    if (result)
    {
        cout << host << " connected on " << svc << endl;
    }
    else
    {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on " << ntohs(client.sin_port) << endl;
    }
    // Create DB
    map <int, string> storage;
    // While receiving- display message, echo message
    char buf[4096];
    while (true)
    {
        // Clear the buffer
        memset(buf, 0, 4096);
        // Wait for message
        int byteRecv = recv(clientSocket, buf, 4096, 0);
        if (byteRecv == -1)
        {
            cerr << "There was a connection issue" << endl;
            break;
        }
        if (byteRecv == 0)
        {
            cout << "The client disconnected" << endl;
            break;
        }

        // Display message
        cout << "Reseived: " << string(buf, 0, byteRecv) << endl;

        
        vector<string> arr;
        string str (string(buf, 0, byteRecv));
        string separat(" ");
        size_t prev = 0;
        size_t next;
        size_t delta = separat.length();

        while((next = str.find(separat, prev)) != string::npos)
        {
            
            string tmp = str.substr(prev, next-prev);
            // cout << tmp << endl;
            
            arr.push_back(str.substr(prev, next-prev));
            prev = next + delta;

            string arr[] = {tmp};
        }
        
        string tmp = str.substr(prev);
        // cout << tmp << endl;
       
        arr.push_back(str.substr(prev));

        string operation = arr[0];
        string str_key  = arr[1];
        int key = stoi(str_key);
        string value = tmp;

        stringstream response;

        if (operation == "get") 
        {

            auto it = storage.find(key);
                if (it == storage.end()) 
                {        
                   response << "not found";
                } else
                {
                    response << it->second;
                } 
        } else if (operation == "set")
        {
            storage[key] = value;
            response << "saved successfully";
        }

        // Resend message
        send(clientSocket, response.str().c_str(),
        response.str().length(), 0);
        
        
        // send(clientSocket, buf, byteRecv + 1, 0);
    }
    // Close socket
    close(clientSocket);
    return 0;
}