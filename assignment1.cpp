#include <fstream>
#include <iostream>
#include <unordered_map>
#include <iosfwd>
#include <list>

// using C++ 20
using namespace std;

unordered_map<string, list<pair<string,string>>::iterator> dns_map; // map for the domains and ips
list<pair<string, string>> dns_list; //doubly linked list
int capacity = 5; // capacity of list

string get_ip_address_from_file(const string& domain_name) {
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

void addValue(string& domain, string& ip) { // function that deals with the linked list and LRU additions
    if (dns_list.size() == capacity) { // if the size of the list is over capacity, erase the LRU
        dns_map.erase(dns_list.back().first); // erase it from the map
        dns_list.pop_back(); // remove it from the list
    }
    dns_list.emplace_front(domain, ip); // constructs the element at the front of the list
    dns_map[domain] = dns_list.begin();
}

string get_ip_address(const string& domain_name) {
    if (dns_map.contains(domain_name)) { // if the map already contains the domain
        dns_list.splice(dns_list.begin(), dns_list, dns_map[domain_name]);
        return dns_map[domain_name] -> second; // return the value
    } // ELSE
    string ip = get_ip_address_from_file(domain_name);
    string domain = domain_name;
    if (ip.empty()) { // if get_ip_address_file returned an empty string
        cout << "No IP address found" << endl;
        return ip;
    }
    addValue(domain, ip); //otherwise put the domain and ip into the cache as a pair
    return ip;
}

void print_cache(void) {
    list<pair<string, string>>::iterator itr; // uses an iterator to go through all the pairs in the cache and print them
    for (itr = dns_list.begin(); itr != dns_list.end(); itr++) {
        cout << itr-> first << "  " << itr-> second << endl; // prints out the pairs
    }
}

void print_dns_file(string filename) { // prints every line in the cache
    ifstream myfile("dns.txt");
    if (myfile.is_open()) {
        string line;
        while (getline(myfile,line)) {
            cout << line << endl; // prints each line
        }
    }
    myfile.close();
}

void clean_up_cache(string filename) {
    ifstream file(filename); // open the file
    unordered_map<string,string> map2; // makes another map to check if every domain and is the same in cache
    if (file.is_open()) {
        string line;
        while (getline(file,line, '=')) {
            string value;
            getline(file,value);
            map2[line] = value;
        }
        file.close();
    } // puts new dns pairs from the file into a new map

    list<pair<string, string>>::iterator itr; // makes an iterator to go through the dns_list
    for (itr = dns_list.begin(); itr != dns_list.end(); itr++) {
        if (map2.contains(itr->first) && map2[itr->first] != itr->second) { //if the domain name exists, but the ip is not the same
            addValue(itr->first, itr->second);
        }
        else if (!map2.contains(itr->first)) { // if the domain doesn't exist in the dns file
            dns_map.erase(itr->first); // erase it from the list
            itr = dns_list.erase(itr); // change the iterator
        }
    }
}

void add_update(string filename){
    string key ;
    string value;
    cout << "input a domain name" << endl; // request input
    cin >> key;
    cout << "input a ip address" << endl; // request input again
    cin >> value;
    if (dns_map.contains(key)) { // if the cache has the key
        dns_map[key] -> second = value;
        ofstream file("dns.txt", ios::trunc);
        list<pair<string, string>>::iterator itr;
        for (itr = dns_list.begin(); itr != dns_list.end(); itr++) { // goes through the cache and puts it into the file
            file << itr -> first << "=" << itr-> second << endl;
        }
        file.close();
    }
    else { // if the cache does not contain the key
        ofstream file(filename, ios::app);
        file << key << "=" << value << endl; // this appends the new domain and ip to the file
        file.close();
    }
}

int main() {
    // Fill the cache to its maximum capacity
    std::cout << "IP address for www.example.com: " << get_ip_address("www.example.com") << std::endl;
    std::cout << "IP address for www.google.com: " << get_ip_address("www.google.com") << std::endl;
    std::cout << "IP address for www.github.com: " << get_ip_address("www.github.com") << std::endl;
    std::cout << "IP address for www.stackoverflow.com: " << get_ip_address("www.stackoverflow.com") << std::endl;
    std::cout << "IP address for www.reddit.com: " << get_ip_address("www.reddit.com") << std::endl;

    // Print the cache contents
    print_cache();

    // Access an existing entry to update its position in the LRU list
    std::cout << "IP address for www.google.com: " << get_ip_address("www.google.com") << std::endl;

    // Add a new entry, which should evict the least recently used entry
    std::cout << "IP address for www.twitter.com: " << get_ip_address("www.twitter.com") << std::endl;
        // twitter does not have a related ip address so it doesn't push anything out

    // Print the cache contents again to verify the eviction
    print_cache();

    // Modify the DNS file to simulate changes
    std::ofstream dnsFile("dns.txt", std::ios::trunc);
    dnsFile << "www.example.com=192.168.1.101" << std::endl;
    dnsFile << "www.google.com=8.8.8.8" << std::endl;
    dnsFile << "www.github.com=192.30.255.113" << std::endl;
    dnsFile << "www.stackoverflow.com=151.101.1.69" << std::endl;
    dnsFile << "www.reddit.com=151.101.1.70" << std::endl;
    dnsFile.close();

    // Clean up the cache
    clean_up_cache("dns.txt");

    // Print the cache contents again to verify the cleanup
    print_cache();

    // Print the contents of the DNS file
    print_dns_file("dns.txt");

    // Test the add_update function
    add_update("dns.txt");

    // Print the cache and DNS file contents again to verify the changes
    print_cache();
    print_dns_file("dns.txt");

    return 0;

}