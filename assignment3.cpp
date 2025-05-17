#include <iostream>
#include <fstream>
#include <iosfwd>
#include <map>
#include <sstream>
#include <vector>

// using C++ 20
using namespace std;

class DNSManager {
public:
    string filename;
    DNSManager() = default;

    virtual ~DNSManager() = default;

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
            ifstream myfile (filename);
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
        ifstream myfile(filename);
        if (myfile.is_open()) {
            string line;
            while (getline(myfile,line)) {
                cout << line << endl; // prints each line
            }
        }
        myfile.close();
    }

    virtual string getIPAddressFromFile(const string& domain_name) { //gets the IP address from the Dns File
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
        else if (!file.is_open()) {
            throw "File not found";
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
class DistributedDNSManager : public DNSManager {
    // have 3 different text files for alphabets
    // dns1.txt A-I
    // dns2.txt J-R
    // dns3.txt S-Z
    // split and do proper functions to proper txt files

    public:
    virtual void splitDNS(const string& filename) {
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file,line)) {  //splits the line according to the deliminator
                string rightfile = whichfile(line);
                ofstream outfile(rightfile, ios::app); // write to the right txt file
                outfile << line << endl; // puts that line into the appropriate file
                outfile.close();
            }
        }
        file.close();
    }

    virtual string whichfile(string domain_name) { // returns which file i should be opening and writing to lmao
        char first = tolower(domain_name[0]);
        if (first >= 'a' && first <= 'i') return "dns1.txt";
        if (first >= 'j' && first <= 'r') return "dns2.txt";
        return "dns3.txt";
    }

    string getIPAddressFromFile(const string &domain_name) override {
        string filename = whichfile(domain_name);
        ifstream file(filename);
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
        else if (!file.is_open()) {
            throw "File not found";
        }
        file.close();
        return  ""; // return empty string if the domain name doesn't exist
    }


};

class EnhancedDistributedDNSManager : public DistributedDNSManager {
    public:
    map <string,string> domainandfile; // to keep track of where each domain ended up
    int dns1 = 0;
    int dns2 = 0;
    int dns3 = 0;

    void splitDNS(const string &filename) override {
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file,line)) {
                string chosenfile = smallest();
                ofstream outfile(chosenfile, ios::app); // write to the right txt file
                outfile << line << endl; // puts that line into the appropriate file
                outfile.close();
                string domain;
                stringstream fullLine(line);
                getline(fullLine ,domain, '=');
                domainandfile[domain] = chosenfile; // maps the domain name to the chosenfile
            }
        }
        else {
            throw "File not found";
        }
    }

    string smallest () {
        string files[3] = {"dns1.txt","dns2.txt","dns3.txt"};
        int min = INT_MAX;
        string chosenFile;
        for (string file : files) {
            ifstream fileIn(file);
            int count = 0;
            string line;
            while (getline(fileIn,line)) {
                count++;
            }
            if (count < min) {
                min = count;
                chosenFile = file;
            }
        }
        return chosenFile; // this function returns the file with the least amount of lines
    }

    string getIPAddressFromFile(const string &domain_name) override {
        string filename = whichfile(domain_name);
        ifstream file(filename);
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
        else if (!file.is_open()) {
            throw "File not found";
        }
        file.close();
        return  ""; // return empty string if the domain name doesn't exist
    }

    string whichfile(string domain_name) override {
        string filename = domainandfile[domain_name];
        if (filename.empty()) {
            throw "Domain not found";
        }
        return filename;
    }
};

class CacheManager {
    DNSManager* dnsManager; // private dnsManager

public:
    int cap; //capacity
    Node* head;
    Node* tail;


    CacheManager(int cap, DNSManager* manager) {
        this->cap = cap;
        this->head = nullptr;
        this->tail = nullptr;
        this->dnsManager = manager;
    }

    virtual ~CacheManager() = default;

    virtual string getIPAddress(const string &domain_name) {
        if (contains(domain_name)) { // if the cache already contains the domain
            Node* temp = head;
            while (temp != nullptr) {
                if (temp->domain == domain_name) {
                    remove(temp->domain);
                    moveToFront(temp->domain, temp->ip); // move it to the front
                    return temp->ip; //return the value
                }
                temp = temp->next;
            }
        } // ELSE
        string ip = dnsManager->getIPAddressFromFile(domain_name);
        string domain = domain_name;
        if (ip.empty()) { // if get_ip_address_file returned an empty string
            cout << "no IP address found" << endl;
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
            string ipaddress = dnsManager->getIPAddressFromFile(current->domain);
            if (ipaddress.empty()) { // if the domain and ip no longer exist
                Node* temp = current->next;
                Node* toRemove = remove(current->domain); // change pointers and remove node
                if (toRemove == nullptr) {
                    throw "couldn't find node to remove";
                }
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
        dnsManager->addUpdate(filename);
    }
    void printDNSFile(string filename) { // same as above
        dnsManager->printDNSFile(filename);
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

    virtual void addValue(string& domain, string& ip) { // function that deals with the linked list and LRU additions
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

    virtual void removeLast() { // helper for remove last
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

class LFU : public CacheManager { // LFU class inheriting from the main cache manager
    DNSManager* dnsManager; // private dnsManager
public:
    map<string,int> frequency;

    LFU (int cap, DNSManager* manager): CacheManager(cap, dnsManager) {
        this->cap = cap;
        this->head = nullptr;
        this->tail = nullptr;
        dnsManager = manager;
    }

    // when accessed, i need to update the number in frequency
    // when removing, take the one with the lowest frequency
    // WHAT AM I DOING???
    string getIPAddress(const string &domain_name) override {
        if (contains(domain_name)) { // if the cache already contains the domain
            Node* temp = head;
            while (temp != nullptr) {
                if (temp->domain == domain_name) {
                    frequency[temp->domain]++; // up the frequency
                    return temp->ip; //return the value
                }
                temp = temp->next;
            }
        } // ELSE
        string ip = dnsManager->getIPAddressFromFile(domain_name);
        string domain = domain_name;
        if (ip.empty()) { // if get_ip_address_file returned an empty string
            cout << "no IP Address found" << endl;
            return ip;
        }
        frequency[domain_name] = 0;
        addValue(domain, ip); // otherwise put the domain and ip into the front of the linked list
        return ip;
    }

    void addValue(string &domain, string &ip) override {
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
    }

    void removeLast() override { // instead of removing the last, it'll be removing the LFU
        Node* current = head;
        int leastfrequency = INT_MAX;
        Node* leastfrequencyNode = head;
        while (current != nullptr) {
            if (frequency[current->domain] < leastfrequency) {
                leastfrequency = frequency[current->domain];
                leastfrequencyNode = current;
            }
            current = current->next;
        }
        delete remove(leastfrequencyNode->domain);
    }
};

class LIFO : public CacheManager { // LIFO inheriting from the main cache manager
    // changing stack position based on when it was accessed, instead of removing the last one, we remove the first one
    DNSManager* dnsManager; // private dnsManager

    public:
    LIFO (int cap, DNSManager* manager): CacheManager(cap, dnsManager) {
        this->cap = cap;
        this->head = nullptr;
        this->tail = nullptr;
        dnsManager = manager; ;
    }

    string getIPAddress(const string &domain_name) override {
        if (contains(domain_name)) { // if the cache already contains the domain
            Node* temp = head;
            while (temp != nullptr) {
                if (temp->domain == domain_name) {
                    remove(temp->domain);
                    moveToFront(temp->domain, temp->ip); // move it to the front
                    return temp->ip; //return the value
                }
                temp = temp->next;
            }
        } // ELSE
        string ip = dnsManager->getIPAddressFromFile(domain_name);
        string domain = domain_name;
        if (ip.empty()) { // if get_ip_address_file returned an empty string
            cout << "no IP address found" << endl;
            return ip;
        }
        addValue(domain, ip); // otherwise put the domain and ip into the front of the linked list
        return ip;
    }

    void addValue(string &domain, string &ip) override {
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
    }

    void removeLast() override { // it's actually remove first lolol
        if (head == nullptr) {
            return;
        }
        Node* temp = head;
        if (head->next != nullptr) {
            head = head->next;
            head->prev = nullptr;
        } else {
            head = nullptr;
            tail = nullptr;
        }
        delete temp; //destroys dynamically allocated memory -> removes the first
        // Last in, first out
    }
};




int main() {
    cout << "" << endl;
    cout << "thank you for grading my assignment!" << endl;
    cout << "" << endl;
    cout << "My EnhancedDistributedDNSManager sorts domains by the size of the files at entry, when splitting the domains into " << endl;
    cout << "different files it chooses the one with the fewest lines" << endl;
    cout << endl;

    cout << "I also focused on just test cases for the things specifically asked for in A3, " << endl;
    cout <<    "and not every single method in cacheManager and DNSManager" << endl;
    cout << endl;

    try { // my try ... catch for exceptions
        DNSManager dnsManager;
        DistributedDNSManager dnsManager2;
        EnhancedDistributedDNSManager dnsManager3;
        CacheManager cacheManager(5, &dnsManager);
        CacheManager cacheManager2(5, &dnsManager2);
        CacheManager cacheManager3(5, &dnsManager3); // different types of cache managers

        LFU frequency (5, &dnsManager);
        LIFO lastin (5, &dnsManager); // different types of eviction policies

        // Modify the DNS file to simulate changes
        std::ofstream dns("dns.txt", std::ios::trunc);
        dns << "example.com=192.168.1.101" << std::endl;
        dns << "google.com=8.8.8.8" << std::endl;
        dns << "github.com=192.30.255.113" << std::endl;
        dns << "stackoverflow.com=151.101.1.69" << std::endl;
        dns << "reddit.com=151.101.1.70" << std::endl;
        dns << "apple.com=1324.433" << std::endl;
        dns << "zzzz.com=234.2223" << std::endl;
        dns << "yahoo.com=4.3" << std::endl;
        dns << "jack.com=22221.111" << std::endl;
        dnsManager2.splitDNS("dns.txt"); // splits the dns file into 3 separate files
        dns.close();

        cout << "DNS1 lines:" << endl;
        // Print the contents of the DNS file
        cacheManager2.printDNSFile("dns1.txt");
        cout << "" << endl;
        cout << "DNS2 lines:" << endl;
        cacheManager2.printDNSFile("dns2.txt");
        cout << "" << endl;
        cout << "DNS3 lines:" << endl;
        cacheManager2.printDNSFile("dns3.txt");
        remove("dns1.txt");
        remove("dns2.txt");
        remove("dns3.txt");
        cout << "" << endl;
        cout << "this shows alphabetical sorting by domain into 3 different dns files" << endl;
        cout << "please note that they are not alphabetically sorted within said files" << endl;
        cout << endl;
        // Test the add_update function
        //cacheManager.addUpdate("dns.txt");
        // Print the cache and DNS file contents again to verify the changes

        // Modify the DNS file to simulate changes
        std::ofstream dns2("dns.txt", std::ios::trunc);
        dns2 << "example.com=192.168.1.101" << std::endl;
        dns2 << "google.com=8.8.8.8" << std::endl;
        dns2 << "github.com=192.30.255.113" << std::endl;
        dns2 << "stackoverflow.com=151.101.1.69" << std::endl;
        dns2 << "reddit.com=151.101.1.70" << std::endl;
        dns2 << "zoo.com=1525.2353.5" << std::endl;
        dns2 << "jack.com=113545" << std::endl;
        dnsManager3.splitDNS("dns.txt"); // splits the dns file into 3 separate files
        dns2.close();

        cout << "DNS1 lines:" << endl;
        // Print the contents of the DNS file
        cacheManager2.printDNSFile("dns1.txt");
        cout << "" << endl;
        cout << "DNS2 lines:" << endl;
        cacheManager2.printDNSFile("dns2.txt");
        cout << "" << endl;
        cout << "DNS3 lines:" << endl;
        cacheManager2.printDNSFile("dns3.txt");
        remove("dns1.txt");
        remove("dns2.txt");
        remove("dns3.txt");
        cout << "" << endl;
        cout<< "this shows my enhanced dns manager, by splitting them by the file with minimum lines" << endl;
        cout << "because they're no longer sorted by alphabet, there's a map storing which file" << endl;
        cout << "each domain is stored in" << endl;
        cout << "" << endl;


        // now we just need to test LFU and LIFO
        cout << "this is my tester for LFU:" << endl;
        cout << "" << endl;

        std::ofstream lfu("dns.txt", std::ios::trunc);
        lfu << "www.example.com=192.168.1.101" << std::endl;
        lfu << "www.google.com=8.8.8.8" << std::endl;
        lfu << "www.github.com=192.30.255.113" << std::endl;
        lfu << "www.stackoverflow.com=151.101.1.69" << std::endl;
        lfu << "www.reddit.com=151.101.1.70" << std::endl;
        lfu << "www.twitter.com=14423.1124" << std::endl;
        lfu.close();


        std::cout << "IP address for www.example.com: " <<
        frequency.getIPAddress("www.example.com") << std::endl;
        std::cout << "IP address for www.google.com: " <<
        frequency.getIPAddress("www.google.com") << std::endl;
        std::cout << "IP address for www.github.com: " <<
        frequency.getIPAddress("www.github.com") << std::endl;
        std::cout << "IP address for www.stackoverflow.com: " <<
        frequency.getIPAddress("www.stackoverflow.com") << std::endl;
        std::cout << "IP address for www.reddit.com: " <<
        frequency.getIPAddress("www.reddit.com") << std::endl;
        cout << "" << endl;
        // Print the cache contents
        cout << "this is the cache contents" << endl;
        frequency.printCache();
        cout << "" << endl;
        // Access an existing entry to update its frequency
        frequency.getIPAddress("www.google.com");
        frequency.getIPAddress("www.github.com");
        frequency.getIPAddress("www.stackoverflow.com");
        frequency.getIPAddress("www.reddit.com");
        cout<< "visit all other domains" << endl;

        cout << "" << endl;
        cout << "example.com should be the cache element that gets evicted since i've accessed all the other ones once" << endl;
        cout << "" << endl;
        // Add a new entry, which should evict the least recently used entry

        frequency.getIPAddress("www.twitter.com");
        // Print the cache contents again to verify the eviction
        frequency.printCache();
        cout << "" << endl;
        cout << "and it is the one that gets evicted!" << endl;
        cout << "please note that the ordering in the cache is still based on LRU, but there is an" << endl;
        cout << "extra data structure that maintains the frequency" << endl;
        cout << " " << endl;
        // Print the contents of the DNS file
        cout << "this is just printing the dns file" << endl;
        frequency.printDNSFile("dns.txt");
        remove("dns.txt");

        cout << "" << endl;
        cout << "now this is my tester for LIFO:" << endl;
        cout << "" << endl;


        std::ofstream lifo("dns.txt", std::ios::trunc);
        lifo << "www.example.com=192.168.1.101" << std::endl;
        lifo << "www.google.com=8.8.8.8" << std::endl;
        lifo << "www.github.com=192.30.255.113" << std::endl;
        lifo << "www.stackoverflow.com=151.101.1.69" << std::endl;
        lifo << "www.reddit.com=151.101.1.70" << std::endl;
        lifo << "www.twitter.com=14423.1124" << std::endl;
        lifo.close();


        std::cout << "IP address for www.example.com: " <<
        lastin.getIPAddress("www.example.com") << std::endl;
        std::cout << "IP address for www.google.com: " <<
        lastin.getIPAddress("www.google.com") << std::endl;
        std::cout << "IP address for www.github.com: " <<
        lastin.getIPAddress("www.github.com") << std::endl;
        std::cout << "IP address for www.stackoverflow.com: " <<
        lastin.getIPAddress("www.stackoverflow.com") << std::endl;
        std::cout << "IP address for www.reddit.com: " <<
        lastin.getIPAddress("www.reddit.com") << std::endl;
        cout << "" << endl;
        // Print the cache contents
        cout << "this is the cache contents" << endl;
        lastin.printCache();
        cout << "" << endl;
        // Access an existing entry to update it's place in the stack
        lastin.getIPAddress("www.google.com");
        cout<< "visit google so it's at the top" << endl;
        lastin.printCache();
        cout << "here's the cache after visiting" << endl;
        cout << "" << endl;
        cout << "google.com should be the one evicted since i visited it the most recently" << endl;
        cout << "" << endl;
        // Add a new entry, which should evict the least recently used entry

        lastin.getIPAddress("www.twitter.com");
        // Print the cache contents again to verify the eviction
        lastin.printCache();
        cout << "" << endl;
        cout << "and it is the one that gets evicted!" << endl;
        cout << "since it was last visited, it's the first out" << endl;
        cout << " " << endl;
        // Print the contents of the DNS file
        cout << "this is just printing the dns file" << endl;
        lastin.printDNSFile("dns.txt");
        remove("dns.txt");

        cout << "" << endl;
        cout << "i can't really show exception catching in the main since it stop the code" << endl;
        cout << "but there are points in my code where i throw errors" << endl;
        cout << "and then in the main, i catch my specific ones, and I catch standard ones" << endl;
        cout << "" << endl;
        cout << "for example:" << endl;
        cout << "here's a case where i try to get an ip address with a bogus domain name, which then" << endl;
        cout << "propagates to not being able to find the file" << endl;
        cout << "" << endl;
        lastin.getIPAddress("dns.file");

    }
    catch(const char* message) {
        cout << message << endl; // my thrown exceptions
    }
    catch (const exception& e) {
        cout << e.what() << endl; // all other standard exceptions
    }

    return 0;
}

