#include <iostream>
#include <fstream>
#include <iosfwd>
#include <vector>

// using C++ 20
using namespace std;

class DNSManager {
public:
    string filename;
    DNSManager() = default;
    ~DNSManager() = default;
    void addUpdate(string filename) {
        string key ;
        string value;
        cout << "input a domain name" << endl; // request input
        cin >> key;
        cout << "input a ip address" << endl; // request input again
        cin >> value;

        string replace = getIPAddressFromFile(key);
        string line;
        if (replace.empty()) { // if the domain doesn't exit
            ofstream file(filename, ios::app);
            file << key << "=" << value << endl; // this appends the new domain and ip to the file
            file.close();
        }
        else if (replace != value) { // else if the domain exists, but it's different
            ifstream myfile ("dns.txt");
            vector<string> lines; // TA in ed discussion said it was okay to use vectors :D
            while (getline(myfile, line)) {
                size_t pos = line.find("=");
                if (pos != string::npos) {
                    string newkey = line.substr(0, pos);
                    if (key == newkey) { //checks if the key in that spot is the same as the given key
                        line = key + "=" + value; // if true, then change the IP address
                    }
                }
                lines.push_back(line);
            }
            myfile.close();

            ofstream file(filename, ios::trunc);
            for (const string& updatedLine : lines) {
                file << updatedLine << endl; // write to the file
            }
            file.close();

        }

    }
    static void printDNSFile(const string& filename) { // prints every line in the dnsFile
        ifstream myfile("dns.txt");
        if (myfile.is_open()) {
            string line;
            while (getline(myfile,line)) {
                cout << line << endl; // prints each line
            }
        }
        myfile.close();
    }
    static string getIPAddressFromFile(const string& domain_name) { //gets the IP address from the Dns File
        ifstream file("dns.txt");
        if (file.is_open()) {
            string line;
            while (getline(file,line, '=')) {  //splits the line according to the deliminator
                string value;
                getline(file >> ws,value); // gets just the number
                if (line == domain_name) { // if the split line is equal to the domain
                    file.close();
                    return value; //return the ip
                }
            }
        }
        file.close();
        return  ""; // return empty string if the domain name doesn't exist
    }

};

class Node { // Class for node implementation
    // no one said this wasn't okay so I hope it's fine!
    public:
    string domain;
    string ip;
    Node* next;
    Node* prev;

    Node(string& domain, string& ip) {
        this->domain = domain;
        this->ip = ip;
        this->next = nullptr;
        this->prev = nullptr;
    }
};

class CacheManager {
    DNSManager dnsManager; // private dnsManager

public:
    int cap; //capacity
    Node* head;
    Node* tail;


    CacheManager(int cap) {
        this->cap = cap;
        this->head = nullptr;
        this->tail = nullptr;
        dnsManager = DNSManager();
    }

    ~CacheManager() = default;

    string getIPAddress(const string &domain_name) {
        if (contains(domain_name)) { // if the cache already contains the domain
            Node* temp = head;
            while (temp != nullptr) {
                if (temp->domain == domain_name) {
                    remove(temp->domain);
                    moveToFront(temp->domain, temp->ip);
                    return temp->ip; //return the value
                }
                temp = temp->next;
            }
        } // ELSE
        string ip = dnsManager.getIPAddressFromFile(domain_name);
        string domain = domain_name;
        if (ip.empty()) { // if get_ip_address_file returned an empty string
            cout << "No IP address found" << endl;
            return ip;
        }
        addValue(domain, ip); // otherwise put the domain and ip into the front of the linked list
        return ip;
    }

    void printCache() { //prints the cache
        Node* current = head;
        while (current != nullptr) {
            cout << current->domain << "=" << current->ip << endl;
            current = current->next;
        }
    }

    void cleanUpCache(string filename) { //cleans up the cache!
        Node* current = head;
        while (current != nullptr) {
            string ipaddress = dnsManager.getIPAddressFromFile(current->domain);
            if (ipaddress.empty()) { // if the domain and ip no longer exist
                Node* temp = current->next;
                Node* toRemove = remove(current->domain); // change pointers and remove node
                delete toRemove; // deletes the Node that was returned for destroying
                current = temp; // move to next node
            }
            else if (ipaddress != current->ip) { // if the found ip is not the same as it is in the cache
                current->ip = ipaddress; // change in place
                current = current->next; // move to next node
            }

            else {
                current = current->next;
            }

        }

    }
    void addUpdate(string filename) { // calls the function from the dnsManager
        dnsManager.addUpdate(filename);
    }
    void printDNSFile(string filename) { // same as above
        dnsManager.printDNSFile(filename);
    }

    // helper functions for dealing with the doubly linked list
    bool contains(const string& domain) { //checks if the cache contains a certain domain
        Node* current = head;
        while (current != nullptr) {
            if (current->domain == domain) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
    void addValue(string& domain, string& ip) { // function that deals with the linked list and LRU additions
        Node* newNode = new Node(domain, ip); // initializes new node with dynamic memory
        if (size() == cap) { // if the size of the linked list is over capacity, erase the LRU
            removeLast();
        }
        if (size() == 0) { // if the cache is empty
            head = newNode;
            tail = newNode;
        } else {
            Node* temp = head;
            head = newNode;
            head->next = temp;
            temp->prev = newNode;
            newNode->prev = nullptr;
        }
        // adds element to front of doubly linked list
    }
    int size () { //gets size of linked list
        int count = 0;
        Node* current = head;
        while (current != nullptr) {
            count++;
            current = current->next;
        }
        return count;
    }

    void removeLast() { // helper for remove last
        if (tail == nullptr) {
            return;
        }
        Node* temp = tail;
        if (tail-> prev != nullptr) {
            tail = tail->prev;
            tail->next = nullptr;
        } else {
            head = nullptr;
            tail = nullptr;
        }
        delete temp; //destroys dynamically allocated memory
    }

    Node* remove(string& domain) { // removes a designated node
        Node* current = head;
        while (current != nullptr) {
            if (current->domain == domain) { // if the domains match
                if (current == head) { // if it's the head
                    head = current->next;
                    head->prev = nullptr;
                } else if (current == tail) { // if it's the tail
                    tail = current->prev;
                    tail->next = nullptr;
                }
                else { // otherwise
                    Node* temp = current->prev;
                    Node* tempn = current->next;
                    temp->next = tempn;
                    tempn->prev = temp;
                }
                return current; //returns the node that needs to be destroyed
            }
            current = current->next;
        }
        return nullptr; //if it couldn't be found, return nothing
    }
    void moveToFront(string& domain, string& ip) {
        Node* current = head;
        Node* newNode = new Node(domain, ip);
        head = newNode;
        newNode->next = current;
        current->prev = newNode;
    }

};





int main() {
    CacheManager cacheManager(5);
    // Fill the cache to its maximum capacity
    std::cout << "IP address for www.example.com: " <<
    cacheManager.getIPAddress("www.example.com") << std::endl;
    std::cout << "IP address for www.google.com: " <<
    cacheManager.getIPAddress("www.google.com") << std::endl;
    std::cout << "IP address for www.github.com: " <<
    cacheManager.getIPAddress("www.github.com") << std::endl;
    std::cout << "IP address for www.stackoverflow.com: " <<
    cacheManager.getIPAddress("www.stackoverflow.com") << std::endl;
    std::cout << "IP address for www.reddit.com: " <<
    cacheManager.getIPAddress("www.reddit.com") << std::endl;
    // Print the cache contents
    cacheManager.printCache();
    // Access an existing entry to update its position in the LRU list
    std::cout << "IP address for www.google.com: " <<
    cacheManager.getIPAddress("www.google.com") << std::endl;
    // Add a new entry, which should evict the least recently used entry
    std::cout << "IP address for www.twitter.com: " <<
    cacheManager.getIPAddress("www.twitter.com") << std::endl;
    // Print the cache contents again to verify the eviction
    cacheManager.printCache();
    cout << " " << endl;
    // Modify the DNS file to simulate changes
    std::ofstream dnsFile("dns.txt", std::ios::trunc);
    dnsFile << "www.example.com=192.168.1.101" << std::endl;
    dnsFile << "www.google.com=8.8.8.8" << std::endl;
    dnsFile << "www.github.com=192.30.255.113" << std::endl;
    dnsFile << "www.stackoverflow.com=151.101.1.69" << std::endl;
    dnsFile << "www.reddit.com=151.101.1.70" << std::endl;
    dnsFile.close();
    // Clean up the cache
    cacheManager.cleanUpCache("dns.txt");
    // Print the cache contents again to verify the cleanup
    cacheManager.printCache(); // it printed the cache twice?
    cout << " " << endl;
    // Print the contents of the DNS file
    cacheManager.printDNSFile("dns.txt");
    // Test the add_update function
    cacheManager.addUpdate("dns.txt");
    // Print the cache and DNS file contents again to verify the changes
    cacheManager.printCache();
    cout << " " << endl;
    cacheManager.printDNSFile("dns.txt");
    return 0;
}