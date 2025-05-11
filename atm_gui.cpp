
//including libraries of wxwdigets
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <iostream>         
#include <string>
#include <vector>
#include <ctime>            
#include <stdexcept>        
#include <memory>          
#include <fstream>          
#include <iomanip>          
#include <map>              
#include <windows.h>
#include <wx/log.h>       
#include <wx/msgdlg.h>      
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/stattext.h>   
#include <wx/textctrl.h>    
#include <wx/listbox.h>     
#include <wx/sizer.h>      
#include <wx/statbmp.h>     
#include <wx/image.h>      
#include <wx/settings.h>    
#include <wx/font.h>        
#include <wx/colour.h>     
using namespace std;

//setting up colors
const wxColour BAH_GREEN(0, 105, 56);           
const wxColour BAH_WHITE(255, 255, 255);
const wxColour BAH_TEXT_ON_GREEN = BAH_WHITE;   
const wxColour INPUT_BG(255, 255, 255);         //background color for text display 
const wxColour INPUT_FG(0, 0, 0);               //text color for text display
// Define unique ids for GUI controls 
const int ID_KEYPAD_BASE = 1000;                
const int ID_OPTIONS_BASE = 1100;              
const int ID_BTN_WITHDRAW = ID_OPTIONS_BASE + 0;
const int ID_BTN_BALANCE = ID_OPTIONS_BASE + 1;
const int ID_BTN_HISTORY = ID_OPTIONS_BASE + 2;
const int ID_BTN_EXIT = ID_OPTIONS_BASE + 3;
const int ID_BTN_ACCOUNT_TYPE_BASE = 1200;      
const int ID_BTN_CURRENT = ID_BTN_ACCOUNT_TYPE_BASE + 0;
const int ID_BTN_SAVINGS = ID_BTN_ACCOUNT_TYPE_BASE + 1;
const int ID_QUICK_CASH_BASE = 1300;            
const int ID_QC_500 = ID_QUICK_CASH_BASE + 0;
const int ID_QC_1000 = ID_QUICK_CASH_BASE + 1;
const int ID_QC_10000 = ID_QUICK_CASH_BASE + 2;
const int ID_QC_20000 = ID_QUICK_CASH_BASE + 3;
const int ID_BTN_OTHER_AMOUNT = ID_QUICK_CASH_BASE + 4;
const int ID_BTN_HISTORY_BACK = wxID_HIGHEST + 1; 
const int ID_BTN_BALANCE_BACK = wxID_HIGHEST + 2; 

//linking with the csv file for doing data analysis using python script
const std::string TRANSACTION_LOG_FILE = "transactions.csv";

//implementing classes

class Transaction {
    private:
    string Type;    
    double amount; 
    string date;    

    
    static string getCurrentDate() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
        return string(buffer);
    }

    public:
    
    Transaction(string t, double a) : Type(t), amount(a) { date = getCurrentDate(); }

    
    Transaction(const Transaction& other) : Type(other.Type), amount(other.amount), date(other.date) {}


    string GetType()const{
	    return Type; 
	}
    double GetAmount()const{ 
	    return amount; 
	}
    string GetDate()const{ 
	    return date; 
	}

    wxString GetDisplayString() const {
        return wxString::Format("Date:%s | Type:%s | Amt:$%.2f", date, Type, amount);
    }
};


class BankAccount {
private:
    int AccountNum;            
    double AccountBalance;    
    bool Status;                
    string Pin;                
    
    
    std::vector<Transaction> transactionHistory;
    
    
    //function used for writing the transaction to csv file for record keeping and data analysis
    void logTransactionToCSV(const Transaction& t)const{
    	//logging in append mode
        std::ofstream logFile(TRANSACTION_LOG_FILE, std::ios::app);
        
        if (logFile.is_open()) {
            logFile.seekp(0, std::ios::end); 
            //checking if the file is empty, if empty then write the header
            if (logFile.tellp() == 0) {
                logFile << "Timestamp,AccountNumber,TransactionType,Amount,ResultingBalance\n";
            }
            
            //write the line of transaction hitory detail
            logFile <<std::fixed<< std::setprecision(2)
                    <<t.GetDate()<< ","
                    <<this->AccountNum<< ","
                    <<"\"" << t.GetType()<< "\"," 
                    <<t.GetAmount()<< ","
                    <<this->AccountBalance<< "\n";
            logFile.close();
        } else {
            wxLogError("Failed to open transaction log file: %s", TRANSACTION_LOG_FILE);
        }
    }

public:
    
    BankAccount(int a, double aB, string pin) :
        AccountNum(a), AccountBalance(aB), Status(true), Pin(pin) {
    }

    
    BankAccount() :
        AccountNum(0), AccountBalance(0.0), Status(false), Pin("0000") {}

    
    virtual ~BankAccount() = default;

    //making copy constructor and showing operator overloading necessarily as part of our course
    BankAccount(const BankAccount& other) = default;
    BankAccount& operator=(const BankAccount& other) = default;

    
    void setAccount(int accNum, double accBalance, string pin){
        AccountNum = accNum;
        AccountBalance = accBalance;
        Pin = pin;
        Status = true; 
        transactionHistory.clear(); //clearing previous history
    }

    
    bool authenticate(const string& enteredPin) const {
        return enteredPin == Pin;
    }

    //using getters 
    int getAccountNum()const{ 
	    return AccountNum; 
	}
    double getBalance()const{ 
	    return AccountBalance; 
	}
    bool getStatus()const{ 
	    return Status; 
	}

    
    void deposit(double amount) {
        if (amount <= 0) { wxLogWarning("Deposit amount must be positive."); return; }
        AccountBalance += amount;
        addTransaction("Deposit", amount); 
        wxBell();
        wxLogMessage("Deposited $%.2f | New Balance: $%.2f", amount, AccountBalance);
    }

    virtual bool withdraw(double amount) {
        if (!Status) { wxBell(); wxLogError("Account %d is locked.", AccountNum); return false; }
        if (amount <= 0) { wxLogWarning("Withdrawal amount must be positive."); return false; }
        if (amount > AccountBalance) { wxBell(); wxLogError("Insufficient funds for Account %d!", AccountNum); return false; }
        AccountBalance -= amount;
        addTransaction("Withdrawal", amount); // Log successful withdrawal
        wxBell();
        wxLogMessage("Withdrawn $%.2f | New Balance: $%.2f", amount, AccountBalance);
        return true;
    }

    size_t getTransactionCount()const{ return transactionHistory.size(); }
    const Transaction& getTransaction(size_t index)const{
        return transactionHistory.at(index);
    }

    
    void addTransaction(const string& Type, double a) {
        Transaction t(Type, a);             
        transactionHistory.push_back(t);    //adding vectors
        logTransactionToCSV(t);             
    }

    //handling account status
    void BlockAccount(){ 
	    Status = false; 
		wxLogMessage("Account %d locked", AccountNum);
	}
    void unBlockAccount(){ 
	    Status = true; 
		wxLogMessage("Account %d unlocked", AccountNum); 
	}

    void SetBalance(double b){ 
	    AccountBalance = b; 
	}
};



class Bank {
private:
    //vector array of accounts
    std::vector<std::unique_ptr<BankAccount>>accounts;
    int nextAccountNum; 

public:
    
    Bank() : nextAccountNum(10001222) {}


    BankAccount* createAccount(double initialDeposit, string pin) {
        if (findAccount(nextAccountNum) != nullptr) {
             wxLogError("Error creating account: Account number %d already exists!", nextAccountNum);
             return nullptr;
        }
        accounts.emplace_back(std::make_unique<BankAccount>(nextAccountNum, initialDeposit, pin));
        //using incrementing technique to select nest account number
        int createdAccNum = nextAccountNum++; 
        wxLogInfo("Bank::createAccount successful for Acc %d", createdAccNum);
        return accounts.back().get(); 
    }

    BankAccount* findAccount(int accNum){
        for(const auto& acc_ptr:accounts){
            if(acc_ptr&&acc_ptr->getAccountNum() == accNum){
                return acc_ptr.get(); 
            }
        }
        return nullptr;
    }

    
    void deposit(int accNum, double amount) {
        BankAccount* acc = findAccount(accNum);
        if(acc){
            acc->deposit(amount); 
        }else{
            wxLogWarning("Account %d not found (Bank::deposit)!", accNum);
        }
    }

    
    void withdraw(int accNum, double amount, string pin){
        BankAccount* acc = findAccount(accNum);
        if (acc){
            if(acc->authenticate(pin)){
                acc->withdraw(amount);
            }else{
                wxLogError("Incorrect PIN during withdrawal attempt: Acc %d!", accNum);
                wxBell();
            }
        }else{
            wxLogWarning("Account %d not found (Bank::withdraw)!", accNum);
        }
    }
};

//not needing a user class necessarily but creating for future extension
class User {
private:
    string userName;
    string Pin;
    BankAccount* acc; 
public:
    User(string u, string p, BankAccount* userAcc) : userName(u), Pin(p), acc(userAcc) {}

    int Authenticate(const string& p) const { return (Pin == p); }
    BankAccount* getAccount() const { return acc; }

    void changePIN(const string& oldPin, const string& newPin){
        if(Authenticate(oldPin)) { Pin = newPin; wxLogInfo("User object PIN changed for %s", userName); }
        else{ wxLogError("User object PIN change failed for %s", userName); }
    }
    void deposit(double amount){ if(acc) acc->deposit(amount); } 
    void withdraw(double amount, const string& enteredPin) {
        if(Authenticate(enteredPin)){ if(acc) acc->withdraw(amount); } 
        else{ wxLogError("User withdrawal fail (PIN): %s", userName); }
    }
};


//making a derived class to implement inheritance and polymorphism
class CurrentAccount : public BankAccount {
private:
    double overDraftLimit;
    double minBalance;
    double serviceCharge;
public:
    CurrentAccount(int a, double b, string p, double o, double s) :
        BankAccount(a, b, p), overDraftLimit(o), minBalance(10000.0), serviceCharge(s) {}

    virtual bool withdraw(double amount) override {
        if (!getStatus()) { wxLogError("Account %d locked", getAccountNum()); return false; }
        if (amount <= 0) { wxLogWarning("Withdrawal amount must be positive"); return false; }

        //checking if amount exceeds overdraft limit
        if (amount <= (getBalance() + overDraftLimit)) {
            bool wasOverdraft = (amount > getBalance());
            double currentBalance = getBalance(); 
            SetBalance(currentBalance - amount);
            addTransaction((wasOverdraft ? "Withdrawal (OD)" : "Withdrawal"), amount);
            wxBell();
            wxLogMessage("W/D (Current): $%.2f | New Bal:$%.2f", amount, getBalance());
            return true;
        } else {
            wxLogError("Overdraft limit exceeded for Account %d!", getAccountNum());
            wxBell();
            return false;
        }
    }

    

    void applyServiceCharge() {
        if (getBalance() < minBalance) {
             wxLogWarning("Balance below minimum ($%.2f), service charge skipped for Acc %d", minBalance, getAccountNum());
             return;
        }
        SetBalance(getBalance() - serviceCharge);
        addTransaction("Service Charge", serviceCharge);
        wxLogMessage("Service charge $%.2f applied to Acc %d. New Bal:$%.2f", serviceCharge, getAccountNum(), getBalance());
    }
};

class Savings : public BankAccount {
public:
    Savings(int a, double b, string p) : BankAccount(a, b, p) {}
};

//WX widgets implementation

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};


class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
    virtual ~MyFrame();

private:

    Bank theBank;              
    BankAccount* currentAccount; 

    
    wxPanel *mainPanel;             // Top-level panel holding everything
    wxStaticBitmap* logoCtrl;       // Displays the bank logo
    wxStaticText* displayLabel;    
    wxTextCtrl* inputDisplay;       
    wxPanel* keypadPanel;
    wxPanel* accountTypePanel;
    wxPanel* optionsPanel;
    wxPanel* quickCashPanel;
    wxPanel* historyPanel;
    wxPanel* balancePanel;
    // Buttons for different actions
    wxButton* withdrawBtn;
    wxButton* balanceBtn;
    wxButton* historyBtn;
    wxButton* exitBtn;
    wxButton* currentBtn;
    wxButton* savingsBtn;
    std::map<int, wxButton*> quickCashBtns; 
    wxButton* otherAmountBtn;
    wxListBox* historyList;         
    wxButton* historyBackBtn;
    wxStaticText* currentBalanceLabel;
    wxButton* balanceBackBtn;

    //managing states
    enum ScreenState {              
        STATE_LOGIN_ACCOUNT,        
        STATE_LOGIN_PIN,            
        STATE_ACCOUNT_TYPE_SELECT,  
        STATE_OPTIONS,              
        STATE_WITHDRAW_QUICK_CASH, 
        STATE_WITHDRAW_AMOUNT,      
        STATE_WITHDRAW_CONFIRM_PIN, 
        STATE_VIEW_BALANCE,         
        STATE_VIEW_HISTORY          
    };
    ScreenState currentState;      

    enum AccountMode {             
        MODE_UNSELECTED,
        MODE_CURRENT,
        MODE_SAVINGS
    };
    AccountMode currentMode;        //mod selected for the current session

    
    string enteredInput;            
    string tempAccountNumberStr;    
    double tempWithdrawAmount;      

    //GUI functions
    void SwitchScreen(ScreenState newState); 
    void UpdateDisplay();                    
    void HandleKeypadEnter();                
    void DoLogin();                          
    void PopulateHistory();                  
    void UpdateBalanceDisplay();             

    
    void OnKeypadButton(wxCommandEvent& event);     
    void OnOptionButton(wxCommandEvent& event);   
    void OnBackButton(wxCommandEvent& event);       
    void OnAccountTypeButton(wxCommandEvent& event); 
    void OnQuickCashButton(wxCommandEvent& event);   
    void OnOtherAmountButton(wxCommandEvent& event); 


    wxDECLARE_EVENT_TABLE();
};


wxIMPLEMENT_APP(MyApp); 

// --- MyApp OnInit: Application Entry Point ---
bool MyApp::OnInit() {
    if (!wxApp::OnInit()) return false;

    //to handle all types of image
    wxInitAllImageHandlers();

    MyFrame *frame = new MyFrame("Bank Al Habib ATM (Sim)");
    frame->Show(true); 
    return true; 
}


wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
wxEND_EVENT_TABLE()


//setting up the gui by making panels. took help from chat gpt
MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(450, 700)), 
         currentAccount(nullptr), currentState(STATE_LOGIN_ACCOUNT), currentMode(MODE_UNSELECTED), 
         enteredInput(""), tempWithdrawAmount(0.0)
{

    //creating two accounts to check the system
    theBank.createAccount(50000.0, "1234"); 
    theBank.createAccount(50000.0, "2345"); 

    
    mainPanel = new wxPanel(this, wxID_ANY);
    mainPanel->SetBackgroundColour(BAH_GREEN); 
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL); //choosing vertical layout

    //adding logo
    int logoTargetWidth = 150;
    wxImage logoImage;
    logoCtrl = nullptr; 
    if(logoImage.LoadFile("logo.png") && logoImage.IsOk()){
        int logoTargetHeight = (logoImage.GetHeight() * logoTargetWidth) / logoImage.GetWidth();
        logoImage.Rescale(logoTargetWidth, logoTargetHeight, wxIMAGE_QUALITY_HIGH); 
        logoCtrl = new wxStaticBitmap(mainPanel, wxID_ANY, wxBitmap(logoImage)); 
        mainSizer->Add(logoCtrl, 0, wxALIGN_CENTER | wxALL, 15); 
    }else{
        //error handling
        wxLogError("Failed to load logo.png! Ensure it's in the correct path.");
        wxStaticText* logoFallbackText = new wxStaticText(mainPanel, wxID_ANY, "Bank Al Habib");
        wxFont logoFont = logoFallbackText->GetFont();
        logoFont.SetPointSize(16); logoFont.SetWeight(wxFONTWEIGHT_BOLD);
        logoFallbackText->SetFont(logoFont);
        logoFallbackText->SetForegroundColour(BAH_TEXT_ON_GREEN); 
        mainSizer->Add(logoFallbackText, 0, wxALIGN_CENTER | wxALL, 15);
    }

    displayLabel = new wxStaticText(mainPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    displayLabel->SetFont(displayLabel->GetFont().Larger());         
    displayLabel->SetForegroundColour(BAH_TEXT_ON_GREEN);           
    mainSizer->Add(displayLabel, 0, wxEXPAND | wxALL, 5);           

    inputDisplay = new wxTextCtrl(mainPanel, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 40),
                                  wxTE_READONLY | wxTE_CENTER | wxBORDER_SIMPLE | wxTE_RICH2); 
    inputDisplay->SetFont(wxFontInfo(18).Family(wxFONTFAMILY_TELETYPE)); 
    inputDisplay->SetBackgroundColour(INPUT_BG);                    
    inputDisplay->SetForegroundColour(INPUT_FG);                     
    mainSizer->Add(inputDisplay, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10); 
    keypadPanel = new wxPanel(mainPanel, wxID_ANY);
    keypadPanel->SetBackgroundColour(BAH_GREEN);                    
    wxGridSizer* keypadSizer = new wxGridSizer(4, 3, 5, 5);       
    const char* keypadChars[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "Clear", "0", "Enter"};
    for (int i = 0; i < 12; ++i) {
        wxButton* btn = new wxButton(keypadPanel, ID_KEYPAD_BASE + i, keypadChars[i]);
        btn->SetFont(btn->GetFont().MakeLarger().MakeBold());       
        keypadSizer->Add(btn, 1, wxEXPAND);                         
        btn->Bind(wxEVT_BUTTON, &MyFrame::OnKeypadButton, this);   
    }
    keypadPanel->SetSizer(keypadSizer);
    mainSizer->Add(keypadPanel, 0, wxALIGN_CENTER | wxALL, 10);    

    //making buttons
    accountTypePanel = new wxPanel(mainPanel, wxID_ANY);
    accountTypePanel->SetBackgroundColour(BAH_GREEN);
    wxBoxSizer* typeSizer = new wxBoxSizer(wxVERTICAL);
    currentBtn = new wxButton(accountTypePanel, ID_BTN_CURRENT, "Current Account");
    savingsBtn = new wxButton(accountTypePanel, ID_BTN_SAVINGS, "Savings Account");
    wxFont typeFont = currentBtn->GetFont().MakeLarger();           
    currentBtn->SetFont(typeFont); savingsBtn->SetFont(typeFont);
    typeSizer->Add(currentBtn, 1, wxEXPAND | wxALL, 10);           
    typeSizer->Add(savingsBtn, 1, wxEXPAND | wxALL, 10);
    accountTypePanel->SetSizer(typeSizer);
    mainSizer->Add(accountTypePanel, 1, wxEXPAND | wxALL, 10);      
    currentBtn->Bind(wxEVT_BUTTON, &MyFrame::OnAccountTypeButton, this); 
    savingsBtn->Bind(wxEVT_BUTTON, &MyFrame::OnAccountTypeButton, this);

    //Options Panel
    optionsPanel = new wxPanel(mainPanel, wxID_ANY);
    optionsPanel->SetBackgroundColour(BAH_GREEN);
    wxBoxSizer* optionsSizer = new wxBoxSizer(wxVERTICAL);
    withdrawBtn = new wxButton(optionsPanel, ID_BTN_WITHDRAW, "Withdraw Cash");
    balanceBtn = new wxButton(optionsPanel, ID_BTN_BALANCE, "View Balance");
    historyBtn = new wxButton(optionsPanel, ID_BTN_HISTORY, "View History");
    exitBtn = new wxButton(optionsPanel, ID_BTN_EXIT, "Exit / Logout");
    wxFont optionFont = withdrawBtn->GetFont().MakeLarger();        
    withdrawBtn->SetFont(optionFont); balanceBtn->SetFont(optionFont);
    historyBtn->SetFont(optionFont); exitBtn->SetFont(optionFont);
    optionsSizer->Add(withdrawBtn, 1, wxEXPAND | wxALL, 5);         
    optionsSizer->Add(balanceBtn, 1, wxEXPAND | wxALL, 5);
    optionsSizer->Add(historyBtn, 1, wxEXPAND | wxALL, 5);
    optionsSizer->Add(exitBtn, 1, wxEXPAND | wxALL, 5);
    optionsPanel->SetSizer(optionsSizer);
    mainSizer->Add(optionsPanel, 1, wxEXPAND | wxALL, 10);         

    withdrawBtn->Bind(wxEVT_BUTTON, &MyFrame::OnOptionButton, this);
    balanceBtn->Bind(wxEVT_BUTTON, &MyFrame::OnOptionButton, this);
    historyBtn->Bind(wxEVT_BUTTON, &MyFrame::OnOptionButton, this);
    exitBtn->Bind(wxEVT_BUTTON, &MyFrame::OnOptionButton, this);


    quickCashPanel = new wxPanel(mainPanel, wxID_ANY);
    quickCashPanel->SetBackgroundColour(BAH_GREEN);
    wxGridSizer* quickCashSizer = new wxGridSizer(3, 2, 10, 10);    
    int quickCashAmounts[] = {500, 1000, 10000, 20000};              //amounts that we have set already
    int quickCashIDs[] = {ID_QC_500, ID_QC_1000, ID_QC_10000, ID_QC_20000}; //their ids
    for (int i = 0; i < 4; ++i) {
        wxButton* qcBtn = new wxButton(quickCashPanel, quickCashIDs[i], wxString::Format("$ %d", quickCashAmounts[i]));
        qcBtn->SetFont(qcBtn->GetFont().MakeLarger());
        quickCashSizer->Add(qcBtn, 1, wxEXPAND);                    
        qcBtn->Bind(wxEVT_BUTTON, &MyFrame::OnQuickCashButton, this); 
        quickCashBtns[quickCashIDs[i]] = qcBtn;                    
    }
    quickCashSizer->AddSpacer(0); 
    otherAmountBtn = new wxButton(quickCashPanel, ID_BTN_OTHER_AMOUNT, "Other Amount");
    otherAmountBtn->SetFont(otherAmountBtn->GetFont().MakeLarger());
    quickCashSizer->Add(otherAmountBtn, 1, wxEXPAND);               //addding button
    otherAmountBtn->Bind(wxEVT_BUTTON, &MyFrame::OnOtherAmountButton, this); 
    quickCashPanel->SetSizer(quickCashSizer);
    mainSizer->Add(quickCashPanel, 0, wxALIGN_CENTER | wxALL, 10);  

    historyPanel = new wxPanel(mainPanel, wxID_ANY);
    historyPanel->SetBackgroundColour(BAH_GREEN);
    wxBoxSizer* historySizer = new wxBoxSizer(wxVERTICAL);
    historyList = new wxListBox(historyPanel, wxID_ANY);
    historyList->SetBackgroundColour(INPUT_BG);                     
    historyList->SetForegroundColour(INPUT_FG);                     
    historyBackBtn = new wxButton(historyPanel, ID_BTN_HISTORY_BACK, "Back");
    historySizer->Add(historyList, 1, wxEXPAND | wxALL, 5);        
    historySizer->Add(historyBackBtn, 0, wxALIGN_CENTER | wxALL, 5);
    historyPanel->SetSizer(historySizer);
    mainSizer->Add(historyPanel, 1, wxEXPAND | wxALL, 10);          
    historyBackBtn->Bind(wxEVT_BUTTON, &MyFrame::OnBackButton, this); 


    balancePanel = new wxPanel(mainPanel, wxID_ANY);
    balancePanel->SetBackgroundColour(BAH_GREEN);
    wxBoxSizer* balanceSizer = new wxBoxSizer(wxVERTICAL);
    currentBalanceLabel = new wxStaticText(balancePanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    currentBalanceLabel->SetFont(currentBalanceLabel->GetFont().Larger());
    currentBalanceLabel->SetForegroundColour(BAH_TEXT_ON_GREEN);   
    balanceBackBtn = new wxButton(balancePanel, ID_BTN_BALANCE_BACK, "Back");
    balanceSizer->AddStretchSpacer(1);                            
    balanceSizer->Add(currentBalanceLabel, 0, wxALIGN_CENTER | wxALL, 10);
    balanceSizer->AddStretchSpacer(1);                             
    balanceSizer->Add(balanceBackBtn, 0, wxALIGN_CENTER | wxALL, 5);
    balancePanel->SetSizer(balanceSizer);
    mainSizer->Add(balancePanel, 1, wxEXPAND | wxALL, 10);        
    balanceBackBtn->Bind(wxEVT_BUTTON, &MyFrame::OnBackButton, this); 

    mainPanel->SetSizerAndFit(mainSizer);   
    SwitchScreen(STATE_LOGIN_ACCOUNT);      
    Centre();                               
}

//destructor
MyFrame::~MyFrame(){
    
}


void MyFrame::SwitchScreen(ScreenState newState) {
    wxLogDebug("Switching Screen to state: %d", static_cast<int>(newState));
    currentState = newState;
    enteredInput = ""; 
    UpdateDisplay();   

    keypadPanel->Show(false);
    accountTypePanel->Show(false);
    optionsPanel->Show(false);
    quickCashPanel->Show(false);
    historyPanel->Show(false);
    balancePanel->Show(false);

    bool showKeypad = false;
    long currentInputStyle = inputDisplay->GetWindowStyleFlag();
    inputDisplay->SetWindowStyleFlag((currentInputStyle & ~wxTE_PASSWORD));

    switch (currentState) {
        case STATE_LOGIN_ACCOUNT:
            displayLabel->SetLabel("Enter Account Number:");
            showKeypad = true;
            break;
        case STATE_LOGIN_PIN:
            displayLabel->SetLabel("Enter PIN:");
            inputDisplay->SetWindowStyleFlag(currentInputStyle | wxTE_PASSWORD); 
            showKeypad = true;
            break;
        case STATE_ACCOUNT_TYPE_SELECT:
            displayLabel->SetLabel("Select Account Type:");
            accountTypePanel->Show(true); //show acc type
            break;
        case STATE_OPTIONS:
            if (currentAccount) displayLabel->SetLabel(wxString::Format("Acc: %d (%s) - Select Option:", currentAccount->getAccountNum(), (currentMode == MODE_CURRENT ? "Current" : "Savings")));
            else displayLabel->SetLabel("Error, Not Logged In"); 
            optionsPanel->Show(true); 
            break;
        case STATE_WITHDRAW_QUICK_CASH:
            displayLabel->SetLabel("Select Amount or Choose Other:");
            quickCashPanel->Show(true); 
            break;
        case STATE_WITHDRAW_AMOUNT:
            displayLabel->SetLabel("Enter Amount:");
            showKeypad = true; 
            break;
        case STATE_WITHDRAW_CONFIRM_PIN:
            displayLabel->SetLabel("Re-enter PIN to Confirm:");
            inputDisplay->SetWindowStyleFlag(currentInputStyle | wxTE_PASSWORD); 
            showKeypad = true; 
            break;
        case STATE_VIEW_BALANCE:
            displayLabel->SetLabel("Account Balance");
            UpdateBalanceDisplay(); 
            balancePanel->Show(true); 
            break;
        case STATE_VIEW_HISTORY:
            displayLabel->SetLabel("Transaction History");
            PopulateHistory(); 
            historyPanel->Show(true); 
            break;
        default:
            displayLabel->SetLabel("Error: Unknown State");
            wxLogError("SwitchScreen called with unknown state: %d", static_cast<int>(currentState));
            break;
    }

    keypadPanel->Show(showKeypad); 
    mainPanel->Layout();
}


void MyFrame::UpdateDisplay() {
    if (currentState == STATE_LOGIN_PIN || currentState == STATE_WITHDRAW_CONFIRM_PIN) {
        inputDisplay->SetValue(wxString('*', enteredInput.length())); //showing * for pins
    }else{
        inputDisplay->SetValue(enteredInput); 
    }
}

void MyFrame::HandleKeypadEnter() {
    wxLogDebug("Enter: State %d, Input '%s'", static_cast<int>(currentState), enteredInput);

    switch(currentState) {
    	//handling keypad of every stage
        case STATE_LOGIN_ACCOUNT:
            if (!enteredInput.empty() && enteredInput.length() < 15 && wxString(enteredInput).IsNumber()) {
                tempAccountNumberStr = enteredInput; 
                SwitchScreen(STATE_LOGIN_PIN);
            }else{
                wxLogError("Invalid Account Number format"); wxBell();
                enteredInput = ""; UpdateDisplay(); 
            }
            break;

        case STATE_LOGIN_PIN:
            if (enteredInput.length() == 4 && wxString(enteredInput).IsNumber()) {
                DoLogin(); 
            } else {
                wxLogError("Invalid PIN format (must be 4 digits)."); wxBell();
                enteredInput = ""; UpdateDisplay();
            }
            break;

        case STATE_WITHDRAW_AMOUNT:
            { 
                double amount;
                if (!wxString(enteredInput).ToDouble(&amount) || amount <= 0) {
                     wxLogError("Invalid amount entered. Please enter a positive number."); wxBell();
                     enteredInput = ""; UpdateDisplay();
                } else {
                     tempWithdrawAmount = amount;
                     SwitchScreen(STATE_WITHDRAW_CONFIRM_PIN);
                }
            }
            break;

        case STATE_WITHDRAW_CONFIRM_PIN:
    if (enteredInput.length() == 4 && wxString(enteredInput).IsNumber()) {
        string pinConfirm = enteredInput;
        if (currentAccount && currentAccount->authenticate(pinConfirm)) {
            const double MAX_SAVINGS_W = 25000.0; //setting a limit for savings account
            const double MAX_CURRENT_W = 50000.0; //setting a limit for current account
            bool limitExceeded = false;

            
            if (fmod(tempWithdrawAmount, 500.0) != 0) {
                wxLogError("Withdrawal failed: Amount must be a multiple of 500.");
                limitExceeded = true;
            }

            
            if (!limitExceeded && currentMode == MODE_SAVINGS && tempWithdrawAmount > MAX_SAVINGS_W){
                wxLogError("Withdrawal failed: Amount $%.2f exceeds limit ($%.2f) for Savings mode.", tempWithdrawAmount, MAX_SAVINGS_W);
                limitExceeded = true;
            } else if (!limitExceeded && currentMode == MODE_CURRENT && tempWithdrawAmount > MAX_CURRENT_W) {
                wxLogError("Withdrawal failed: Amount $%.2f exceeds limit ($%.2f) for Current mode.", tempWithdrawAmount, MAX_CURRENT_W);
                limitExceeded = true;
            }

            
            if (!limitExceeded && tempWithdrawAmount > currentAccount->getBalance()){
                 wxLogError("Withdrawal failed: Insufficient funds (Balance: $%.2f).", currentAccount->getBalance());
                 limitExceeded = true;
            }

            
            if (!limitExceeded){
                wxLogInfo("PIN OK & Limits OK. Calling Bank::withdraw for $%.2f", tempWithdrawAmount);
                theBank.withdraw(currentAccount->getAccountNum(), tempWithdrawAmount, pinConfirm);
                UpdateBalanceDisplay(); 
            } else {
                wxBell(); 
            }
        } else {
            wxLogError("Incorrect PIN entered during withdrawal confirmation."); 
            wxBell();
        }
        SwitchScreen(STATE_OPTIONS);
    } else {
        wxLogError("Invalid PIN format entered during confirmation (must be 4 digits)."); 
        wxBell();
        enteredInput = ""; 
        UpdateDisplay(); 
    }
    break;

        default:
             wxLogWarning("Enter pressed in unhandled state: %d", static_cast<int>(currentState));
            break;
    }
}

//using function to validate account number and pin
void MyFrame::DoLogin() {
    long accNumLong;
    //parsing the account number to long int
    if (!wxString(tempAccountNumberStr).ToLong(&accNumLong)) {
        wxLogError("Internal Error: Could not parse account number '%s' during login.", tempAccountNumberStr);
        SwitchScreen(STATE_LOGIN_ACCOUNT); //going back
        tempAccountNumberStr = "";
        return;
    }
    int accNum = static_cast<int>(accNumLong);
    string pin = enteredInput; 

    currentAccount = theBank.findAccount(accNum);
    //if found
    if (currentAccount && currentAccount->authenticate(pin)) {
        wxLogInfo("Login Successful for Account %d", accNum);
        currentMode = MODE_UNSELECTED; 
        SwitchScreen(STATE_ACCOUNT_TYPE_SELECT); 
    }else{
        wxLogError("Login Failed. Invalid Account Number or PIN."); wxBell();
        currentAccount = nullptr; 
        SwitchScreen(STATE_LOGIN_ACCOUNT);
    }
    tempAccountNumberStr = "";
}


void MyFrame::PopulateHistory() {
    historyList->Clear(); //clearing the old history
    //if no account is logged in
    if (!currentAccount) {
        historyList->Append("Error: Not currently logged in.");
        wxLogError("PopulateHistory called while not logged in");
        return;
    }
    size_t count = currentAccount->getTransactionCount(); //getting number of transactions of the current account
    if(count == 0) {
        historyList->Append("No transactions recorded for this account.");
    }else{
        for (size_t i = 0; i < count; ++i){
        	//error handling using try and catch
            try{
                //so here it is accessing in reverse order to display the first one last and last one first
                const Transaction& t = currentAccount->getTransaction(count - 1 - i);
                historyList->Append(t.GetDisplayString());
            }catch(const std::out_of_range& oor){
                wxLogError("Error retrieving transaction at index %d: %s", count - 1 - i, oor.what());
                historyList->Append(wxString::Format("Error retrieving transaction %d", count - 1 - i));
            }
        }
    }
}


void MyFrame::UpdateBalanceDisplay(){
    if (currentAccount){
        currentBalanceLabel->SetLabel(wxString::Format("Current Balance: $%.2f", currentAccount->getBalance()));
    } else {
        currentBalanceLabel->SetLabel("Current Balance: $---.--"); //savings account
    }
}


void MyFrame::OnKeypadButton(wxCommandEvent& event) {
    int id = event.GetId();
    int buttonIndex = id - ID_KEYPAD_BASE; 

    const int MAX_PIN_LEN = 4;
    const int MAX_ACC_LEN = 12; //account limit enetering user
    const int MAX_AMT_LEN = 10; //amount length

    bool blockInput = false;
    //if a user enters account number and then exceeds the limit so it blocks the input
    if ((currentState == STATE_LOGIN_PIN || currentState == STATE_WITHDRAW_CONFIRM_PIN)
        && enteredInput.length() >= MAX_PIN_LEN && buttonIndex != 9 && buttonIndex != 11) {
        blockInput = true;
    }
    if (currentState == STATE_LOGIN_ACCOUNT && enteredInput.length() >= MAX_ACC_LEN && buttonIndex != 9 && buttonIndex != 11) {
        blockInput = true;
    }
    if (currentState == STATE_WITHDRAW_AMOUNT && enteredInput.length() >= MAX_AMT_LEN && buttonIndex != 9 && buttonIndex != 11) {
        blockInput = true;
    }

    if (blockInput) {
        wxBell(); 
        return;  
    }


    if (buttonIndex >= 0 && buttonIndex <= 8) { // Digits 1-9
        enteredInput += to_string(buttonIndex + 1);
    } else if (buttonIndex == 10) { // Digit 0
        //leading zeros error handling
        if(!enteredInput.empty() || currentState == STATE_WITHDRAW_AMOUNT){
            enteredInput += "0";
        } else {
            wxBell();
        }
    }else if(buttonIndex == 9){ //using clear button
        enteredInput = "";
    }else if (buttonIndex == 11){ //using enter button
        HandleKeypadEnter(); 
        return;              
    }else{
        event.Skip();
        return;
    }

    UpdateDisplay();
}



//function checking option selected and based on that performing further
void MyFrame::OnOptionButton(wxCommandEvent& event) {
    int id = event.GetId();
    //security check 
    if (!currentAccount && id != ID_BTN_EXIT) {
        wxLogError("Option button clicked while not logged in! Forcing logout.");
        SwitchScreen(STATE_LOGIN_ACCOUNT);
        return;
    }

    //using switch cases in id and then checking what action to perform
    switch (id) {
        case ID_BTN_WITHDRAW: //1.withdraw
            SwitchScreen(STATE_WITHDRAW_QUICK_CASH); 
            break;
        case ID_BTN_BALANCE:
            SwitchScreen(STATE_VIEW_BALANCE);//2.showing account balance
            break;
        case ID_BTN_HISTORY:
            SwitchScreen(STATE_VIEW_HISTORY);//3.showing transaction history of that particular account
            break;
        case ID_BTN_EXIT:
            wxLogInfo("User logged out via Exit button.");
            currentAccount = nullptr;
            currentMode = MODE_UNSELECTED;
            SwitchScreen(STATE_LOGIN_ACCOUNT);
            break;
        default:
        	//error handling
            wxLogWarning("Unhandled option button ID: %d", id);
            event.Skip(); 
            break;
    }
}


void MyFrame::OnBackButton(wxCommandEvent& event) {
    
    if (currentState == STATE_VIEW_BALANCE || currentState == STATE_VIEW_HISTORY) {
        SwitchScreen(STATE_OPTIONS); 
    }else{
        wxLogWarning("Back button pressed in unexpected state: %d", static_cast<int>(currentState));
        event.Skip();
    }
}


//displaying screen of giving option of selecting account type, savings or current
void MyFrame::OnAccountTypeButton(wxCommandEvent& event){
    int id = event.GetId();
    
    if(id == ID_BTN_CURRENT){
        currentMode = MODE_CURRENT;
        wxLogInfo("-Account Mode set to CURRENT for this session-");
    } 
	else if(id == ID_BTN_SAVINGS){
        currentMode = MODE_SAVINGS;
        wxLogInfo("-Account Mode set to SAVINGS for this session-");
    }else{
        //error handling
        currentMode = MODE_UNSELECTED;
        wxLogWarning("UNKNOWN ACCOUNT TYPE BUTTON PRESSED: %d", id);
    }
    SwitchScreen(STATE_OPTIONS);
}


void MyFrame::OnQuickCashButton(wxCommandEvent& event) {
    int id = event.GetId();
    double amount = 0.0;
    
    switch(id) {
        case ID_QC_500:   
		    amount = 500.0; 
			break;
        case ID_QC_1000:  
		    amount = 1000.0; 
			break;
        case ID_QC_10000: 
		    amount = 10000.0; 
			break;
        case ID_QC_20000: 
		    amount = 20000.0; 
			break;
        default:
            wxLogError("Unknown quick cash button ID received: %d", id);
            return;
    }
    wxLogInfo("QUICK CASH AMOUNT SELECTED : $%.2f", amount);
    //storing the selected amount to proceed further
    tempWithdrawAmount = amount;
    SwitchScreen(STATE_WITHDRAW_CONFIRM_PIN);
}


void MyFrame::OnOtherAmountButton(wxCommandEvent& event) {
    wxUnusedVar(event); 
    
    SwitchScreen(STATE_WITHDRAW_AMOUNT);
}
