#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <iterator>
using namespace std;

// this is a list of all the global variable I used in my project

unordered_map<string,int> webindexing;                      // map website to an index
unordered_map<int,string> webindexingrev;                   // get website name from index

unordered_map<int,vector<int>> adjlist;                    // webgraph (directed)/shows the source city and what it points at
unordered_map<int,vector<int>> transpose;                  // webgraph transpose /shows the destination city and shows all the source cities pointing at it

unordered_map<string, vector<int>> keywords;               // map keyword to websites its in

unordered_map<int, int> impressions;                       // map website index to its number of impressions
unordered_map<int, int> clicks;                            // map website index to its number of clicks
vector<double> PageRank;                                   // stores PageRank of all websites (1-indexed)

vector<int> common(vector<int> &, vector<int> &);

//constructing the graphs...
void ConstructWebgraph();
void LoadKeywords();
void LoadImpressions();
void InitializeClicks();

//searching...
vector<int> searchQuotations(string);
vector<int> ANDsearch(string, string);
vector<int> ORsearch(string, string);
vector<int> searchDefault(vector<string>);
vector<int> search(string);

void InitializePageRank();

//search interaction main...
void final_update();
double pageScore(int i);
void NewSearch();
void printResults(vector<int>);
void website_clicked(vector<int>, int);
void second_UI(vector<int>);
void first_UI();
void InitialText();

int idx = 1;

vector<int> common(vector<int> &a, vector<int> &b){   // find intersection between 2 sorted vectors
    vector<int> answer;
    int i = 0, j = 0;

    while (i < a.size() && j < b.size()){
        if (a[i] == b[j]) {
            answer.push_back(a[i]);
            i++; j++;
        }
        else if (a[i] < b[j]) i++;
        else j++;
    }
    return answer;
}

// loading files

void ConstructWebgraph(){
    ifstream WebgraphInput;
    WebgraphInput.open("Web_graph_file.csv");
    
    if(!WebgraphInput.is_open()) cout << "Error in opening \"Webgraph.csv\"\n";
    
    string line, src, dest;
    
    while (getline(WebgraphInput, line)){
        stringstream str(line);
            
        getline(str, src, ',');
        getline(str, dest, '*');
        
        if(webindexing[src] == 0){       // mapping source to an index
            webindexing[src] = idx;
            webindexingrev[idx] = src;
            idx++;
        }
        
        if(webindexing[dest] == 0){      // mapping destination to an index
            webindexing[dest] = idx;
            webindexingrev[idx] = dest;
            idx++;
        }
        
        adjlist[webindexing[src]].push_back(webindexing[dest]);       // add edge from src to dest in webgraph
        transpose[webindexing[dest]].push_back(webindexing[src]);     // add edge from dest to src in transpose

    }
    
    WebgraphInput.close();
}

void LoadKeywords(){
    ifstream KeywordsInput;
    KeywordsInput.open("Keyword_file.csv");
    
    string line, website, word;
    
    while (getline(KeywordsInput, line)){
        stringstream str(line);
        getline(str, website, ',');
        
        while (getline(str, word, ',')){
                keywords[word].push_back(webindexing[website]);  // push each website to its corresponding keyword
            }
        }

    KeywordsInput.close();
}

void LoadImpressions(){
    ifstream ImpressionsInput;
    ImpressionsInput.open("num_impressions.csv");

    string line, website, impression;

    while(getline(ImpressionsInput, line)){
        stringstream str(line);
        
        getline(str, website, ',');
        getline(str, impression, '*');
        impressions[webindexing[website]] = stoi(impression);    // initialize impressions for website using its index
    }
    
    ImpressionsInput.close();
}

void InitializeClicks(){
    ifstream ClicksInput;
    ClicksInput.open("clicks.csv");
    
    string line, website, click;
    
    while(getline(ClicksInput,line)){
        stringstream str(line);
        
        getline(str, website, ',');
        getline(str, click, '*');
        clicks[webindexing[website]] = stoi(click);    // initialize clicks for website using its index
    }
    
    ClicksInput.close();
}

// searching process

vector<int> searchQuotations(string statement){
    vector<int> answer;
    statement = statement.substr(1,statement.size()-2); // remove quotations
    
    vector<int> :: iterator it;
    for(it = keywords[statement].begin(); it != keywords[statement].end(); it++){
        answer.push_back(*it);
        impressions[*it]++;
    }
    
    return answer;
}

vector<int> ANDsearch(string word1, string word2){
    vector<int> answer, a, b;
    
    vector<int> :: iterator it;
    for(it = keywords[word1].begin(); it != keywords[word1].end(); it++){
        a.push_back(*it);
    }
    for(it = keywords[word2].begin(); it != keywords[word2].end(); it++){
        b.push_back(*it);
    }
    
    answer = common(a,b);
    
    for(int i = 0 ; i < answer.size(); i++){
        impressions[answer[i]]++;
    }
    
    return answer;
}

vector<int> ORsearch(string word1, string word2){
    vector<int> answer;
    set<int> s;
    
    vector<int> :: iterator it;
    for(it = keywords[word1].begin(); it != keywords[word1].end(); it++){
        s.insert(*it);
    }
    for(it = keywords[word2].begin(); it != keywords[word2].end(); it++){
        s.insert(*it);
    }
    
    set<int> :: iterator its;
    for(its = s.begin(); its != s.end(); its++){
        answer.push_back(*its);
        impressions[*its]++;
    }
    
    return answer;
}

vector<int> searchDefault(vector<string> words){ // if no AND or OR..
    vector<int> answer;
    set<int> s;
    
    vector<int> :: iterator it;
    for(int i = 0; i < words.size(); i++){
        for(it = keywords[words[i]].begin(); it != keywords[words[i]].end(); it++){
            s.insert(*it);
        }
    }

    set<int> :: iterator its;
    for(its = s.begin(); its != s.end(); its++){
        answer.push_back(*its);
        impressions[*its]++;
    }
    
    return answer;
}

vector<int> search(string query){
    vector<int> answer;         // vector with websites that contain the required keywords
    
    if(query.front() == '"' && query.back() == '"'){
        answer = searchQuotations(query);
    }
    else{
        vector<string> words;
        string temp = "";
        
        for(int i = 0; i < query.size(); i++){
            if(query[i] == ' '){
                words.push_back(temp);
                temp = "";
            }
            else{
                temp += query[i];
            }
        }
        words.push_back(temp);
 
        if(words[1] == "AND" && words.size() == 3){
            answer = ANDsearch(words.front(), words.back());
        }
        else if (words[1] == "OR" && words.size() == 3){
            answer = ORsearch(words.front(), words.back());
        }
        else{
            answer = searchDefault(words);
        }
    }
    return answer;
}

void InitializePageRank(){
    const int N = idx-1;              // number of nodes in webgraph
    
    vector<double> prev(N+1,1.0/N);
    PageRank.resize(N+1,0.0);           // 1-indexing
    
    vector<int> :: iterator it;
    double MAX = -INT_MIN;
    
    for(int i = 1; i <= 100; i++){      // After I searched on google 100 iterations is to some extent accurate
        for(int j = 1; j <= N; j++){
            for(it = transpose[j].begin(); it != transpose[j].end(); it++){
                PageRank[j] += prev[*it] / adjlist[*it].size();
            }
            MAX = max(MAX,PageRank[j]);
        }
        if(i == 100) break;     // to prevent resetting PageRank before exiting loop
        prev = PageRank;
        PageRank.resize(N+1,0.0);
    }
    
    // normalizing
    for(int i = 1; i <= N; i++){
        PageRank[i] /= MAX;
    }
}

double pageScore(int i){ //according to the video provided by the doctor on the slides
    
    double fraction = (0.1 * impressions[i]) / (1.0 + 0.1 * impressions[i]);
    
    double score = 0.4 * PageRank[i] + ((1.0 - fraction) * PageRank[i] + fraction * (clicks[i] / impressions[i])) * 0.6;
    
    return score;
}

void final_update(){
    
    // updating impressions file
    
    ofstream ImpressionsOutput;
    ImpressionsOutput.open("num_impressions.csv");
    
    for(int i = 1; i < idx - 1; i++){
        ImpressionsOutput << webindexingrev[i] << "," << impressions[i] << "*" << "\n";
    }
    
    ImpressionsOutput.close();
    
    
    // updating clicks file
    
    ofstream ClicksOutput;
    ClicksOutput.open("clicks.csv");
    
    for(int i = 1; i < idx - 1; i++){
        ClicksOutput << webindexingrev[i] << "," << clicks[i] << "*" << "\n";
    }
    
    ClicksOutput.close();

}

void printResults(vector<int> answer){
    
    cout << "\nSearch results: \n";
    if(answer.size() == 0) cout << "No results found\n";
    
    else{
        for(int i = 0; i < answer.size(); i++){
            cout << i+1 << ". " << webindexingrev[answer[i]] << "\n";
        }
    }
}

void NewSearch(){
    
    cout << "Enter query: ";
    string query;
    cin.ignore();
    getline(cin, query,'\n');
        
    vector<int> answer = search(query);
    
    vector<pair<int,int>> temp_answer(answer.size());
    
    for(int i = 0; i < answer.size(); i++){
        temp_answer[i].first = pageScore(answer[i]);
        temp_answer[i].second = answer[i];
    }
    sort(temp_answer.rbegin(),temp_answer.rend());
    
    for(int i = 0; i < answer.size(); i++){
        answer[i] = temp_answer[i].second;
    }
    
    printResults(answer);
    
    second_UI(answer);
}

void website_clicked(vector<int> answer, int index){
    
    clicks[index]++;        // updating clicks for clicked websites
    cout << "\nYou're viewing the contents of \"" << webindexingrev[index] << "\"\n\n";
    
    cout << "choose one of the options:\n";
    cout << "1. Go back to search results page\n";
    cout << "2. New search\n";
    cout << "3. Exit\n\n";
    
    int option;
    
    alternate:
    cout << "Type in your choice: ";
    cin >> option;
    
    if(option == 1){
        printResults(answer);
        second_UI(answer);
    }
    else if(option == 2){
        NewSearch();
    }
    else if (option == 3){
        final_update();
    }
    else if (cin.fail()){                       // prevent an infinite loop if a char or string is inputted instead of int
        cin.clear(); cin.ignore(512, '\n');
        cout << "\n-- Enter a digit --\n\n";
        goto alternate;
    }
    else{
        cout << "\n-- Invalid Choice --\n\n";
        goto alternate;
    }
    
}

void second_UI(vector<int> answer){// after doing the search.. now viewing the results
    
    cout << "\n Choose one of the options:\n";
    cout << "1. Choose a webpage to click on\n";
    cout << "2. New search\n";
    cout << "3. Exit\n\n";
    
    alternate:
    cout << "Type in your choice: ";
    int option;
    cin >> option;
    
    if(option == 1){
        cout << "Type webpage number: ";
        int option;
        cin >> option;
        website_clicked(answer,answer[option-1]);
    }
    else if(option == 2){
        NewSearch();
    }
    else if(option == 3){
        final_update();
    }
    else if (cin.fail()){
        cin.clear(); cin.ignore(512, '\n');
        cout << "\n-- Enter a digit --\n\n";
        goto alternate;
    }
    else{
        cout << "\n-- Invalid Choice --\n\n";
        goto alternate;
    }
}

void first_UI(){

    cout << "Type in your choice: ";
    int option;
    cin >> option;
    
    if(option == 1){
        
        NewSearch();
    }
    
    else if (option == 2){
        final_update();
    }
    else if (cin.fail()){
        cin.clear(); cin.ignore(512, '\n');
        cout << "\n-- Enter a digit --\n\n";
        first_UI();
    }
    else{
        cout << "\n-- Invalid Choice --\n\n";
        first_UI();
    }
}

int main(){
    
    ConstructWebgraph();
    LoadKeywords();
    LoadImpressions();
    InitializeClicks();
    
    InitializePageRank();
    
    cout << "1. New search\n";
    cout << "2. Exit\n\n";
    
    first_UI();
}
