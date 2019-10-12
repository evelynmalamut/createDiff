//
//  createDiff.cpp
//  Project_4
//
//  Created by Evelyn Malamut on 5/31/19.
//  Copyright © 2019 Evelyn Malamut. All rights reserved.
//
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <functional>
#include <utility>
#include <string>
#include <sstream>  // for istringstream and ostringstream
#include <list>
#include <vector>
using namespace std;

struct Seq { //Seq struct (holds sequence and position values
    Seq(string s, int p) //Seq constructor
    : seq(s) {
        positions.push_back(p); //pushes postions into vector
    }
    void addPos(int pos) { //pushes new positions into vector
        positions.push_back(pos);
    }
    string seq;
    vector<int> positions;
};

class HashTable {
public:
    //HashTable(int seq_length)
    HashTable() { //hash table constructor, sets 102400/N buckets to empty lists
        list<Seq> s;
        N = 8;
        for (int i = 0; i < 102400/N; i++) {
            m_buckets.push_back(s);
        }
    }
    
    HashTable(int seq_length) { //hash table constructor, sets 102400/N buckets to empty lists
        list<Seq> s;
        N = seq_length;
        for (int i = 0; i < 102400/N; i++) {
            m_buckets.push_back(s);
        }
    }
    
    void insert(string seq, int pos) //sets bucket to hash of sequence
    {
        vector<int> positions;
        int bucket = hashFunc(seq); //find bucket form hashed string
        if(!search(seq,positions)) { //if sequence doesn't already exist
            m_buckets[bucket].push_back(Seq(seq,pos)); //pushes back the Seq struct into the list at the bucket
        }
        else {
            m_buckets[bucket].begin()->addPos(pos); //if sequence already exists, push position back to first sequence
        }
    }
    
    bool search(string seq, vector<int> &pos) //search for a sequence
    {
        int bucket = hashFunc(seq); //find bucket from hash of sequence
        if (m_buckets[bucket].begin() != m_buckets[bucket].end()) //if it's not an empty list
            for (auto p = m_buckets[bucket].begin(); p != m_buckets[bucket].end(); p++) { //increment through list
                if (p->seq == seq) {//if the sequences are the same
                    pos = p->positions; //set pos to the postions vector
                    return true;
                }
            }
        return false; //if sequence not found, then not in hash table
    }
    
    
private:
    unsigned int hashFunc(const string &hashMe)
    {
        hash<string> str_hash;                      // creates a string hasher
        unsigned int hashValue = str_hash(hashMe);   // hashes string
        
        //gets bucketNum from hash value
        unsigned int bucketNum = hashValue % (102400/N);
        return bucketNum;
    }
    int N; //Length of sequence
    vector<list<Seq>> m_buckets; //vector of lists (open hash table)
};

void createDiff(istream& fold, istream& fnew, ostream& fdiff) {
    char s;
    int N;
    string sold = "";
    string snew = "";
    string add = "";
    vector<int> positions;
    
    //creating string from fold
    while (fold.get(s))
    {
        sold += s;
    }
    
    //setting sequence length size based on size of file (files smaller than 10000 bytes use size 8 and files larger than that use size 16)
    if (sold.size() < 10000) {
        N = 8;
    }
    else {
        N = 16;
    }
    
    HashTable h(N); //create hashtable with certain sequence size
    
    for (int i = 0; i < sold.size() - N; i++) {
        string seq(sold, i, N);
        h.insert(seq,i);
    }
    
    //creating string from fnew
    while (fnew.get(s))
    {
        snew += s;
    }
    
    for (int j = 0; j < snew.size(); j++) { //iterate through string from new file
        if (h.search(snew.substr(j,N),positions)) {//if true, it means that sequence exists in new and old file (and we don't need to add more characters to add instruction)
            int L = 0;
            int pos = 0;
            if (add.size()!= 0) { //testing for non-empty add instruction
                fdiff << "A" + to_string(add.size()) + ":" + add; //send add instruction to fdiff
                add = ""; //set add sequence to empty
            }
            //chooses sequence from position that has longest copy capability
            for (auto p = positions.begin(); p != positions.end(); p++) { //iterate through possible positions
                int temp_pos = *p;
                int k = temp_pos;
                int x = j;
                int temp_L = 0;
                while (k < sold.size() && x < snew.size()) { //iterate through string from old file and from new file
                    if (sold[k] == snew[x]) { //if characters are the same, increase length of copy
                        temp_L++;
                        k++;
                        x++;
                    }
                    else {
                        break; //once the chars stop matching, break from while loop
                    }
                }
                if (temp_L > L) { //if L from current pos is larger than last L
                    L = temp_L; //set L to new L
                    pos = temp_pos; //set pos to current pos
                }
            }
            fdiff << "C" + to_string(L) + "," + to_string(pos); //send copy instruction to fdiff
            j = j + L - 1; //incremenet j to start looking at location after length of copy
        }
        else {
            add += snew.at(j); //add more characters to add instruction
        }
    }
    if (add.size() != 0) { //if not a do-nothing command
        fdiff << "A" + to_string(add.size()) + ":" + add; //instruction to add remaining characters
    }
}

bool getInt(istream& inf, int& n)
{
    char ch;
    if (!inf.get(ch)  ||  !isascii(ch)  ||  !isdigit(ch))
        return false;
    inf.unget();
    inf >> n;
    return true;
}

bool getCommand(istream& inf, char& cmd, int& length, int& offset)
{

    if (!inf.get(cmd))
    {
        cmd = 'x';  // signals end of file
        return true;
    }
    char ch;
    switch (cmd)
    {
        case 'A':
            return getInt(inf, length) && inf.get(ch) && ch == ':';
        case 'C':
            return getInt(inf, length) && inf.get(ch) && ch == ',' && getInt(inf, offset);
        case '\r':
        case '\n':
            return true;
    }
    return false;
}


bool applyDiff(istream& fold, istream& fdiff, ostream& fnew) {
    char s;
    string sold = "";
    string sdiff = "";
    string slength = "";
    string add = "";
    int length = 0;
    int offset = 0;
    char cmd = '\n';
    
    //creating string from fold
    while (fold.get(s))
    {
        sold += s;
    }
    
    //creating string from fdiff
    while (fdiff.get(s))
    {
        sdiff += s;
    }

    fdiff.clear();   // clear the end of file condition
    fdiff.seekg(0);  // reset back to beginning of the stream
    
    while(getCommand(fdiff, cmd, length, offset)) {
        switch(cmd)
        {
            case 'A':
                for(int j = 0; j < length; j++) { //create string of new characters to add
                    fdiff.get(cmd); //get cmd moves the fdiff foward one char
                    add += cmd;
                }
                fnew << add; //send characters to new file
                add = ""; //reset add to empty
                break;
            case 'C':
                for (int j = offset; j < offset + length; j++) { //add characters to string from old file using offset as starting position
                    add += sold.at(j); //add characters from old string to add
                }
                fnew << add; //send characters to new file
                add = ""; //reset add to empty
                break;
            case 'x':
                return true;
        }
    }
    return false;
}
