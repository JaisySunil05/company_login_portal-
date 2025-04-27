#include "crow_all.h"
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

// Base User class
class User {
protected:
    string username, password, name;
public:
    User(const string& uname, const string& pwd, const string& nm)
        : username(uname), password(pwd), name(nm) {}
    virtual ~User() {}
    bool authenticate(const string& uname, const string& pwd) const {
        return (username == uname && password == pwd);
    }
    string getName() const { return name; }
    string getUsername() const { return username; }
};

// Applicant class
class Applicant {
private:
    string name, skills, email, status;
    float cgpa;
public:
    Applicant(const string& nm, float gpa, const string& sk, const string& em)
        : name(nm), cgpa(gpa), skills(sk), email(em), status("Pending") {}
    string toString() const {
        return "Name: " + name + "\nCGPA: " + to_string(cgpa) + "\nSkills: " + skills + "\nEmail: " + email + "\nStatus: " + status;
    }
    float getCGPA() const { return cgpa; }
    string getName() const { return name; }
    string getSkills() const { return skills; }
    string getEmail() const { return email; }
    string getStatus() const { return status; }
};

// Employee class
class Employee : public User {
public:
    Employee(const string& uname, const string& pwd, const string& nm)
        : User(uname, pwd, nm) {}
};

// HR Executive class
class HRExecutive : public User {
public:
    HRExecutive(const string& uname, const string& pwd, const string& nm)
        : User(uname, pwd, nm) {}
};

// Global storage
vector<Applicant> applicants;
vector<Employee> employees = {
    Employee("emp1", "password1", "John Doe"),
    Employee("emp2", "password2", "Jane Smith"),
    Employee("emp3", "password3", "Bob Johnson")
};
vector<HRExecutive> hrExecutives = {
    HRExecutive("hr1", "hrpass1", "Alice Brown"),
    HRExecutive("hr2", "hrpass2", "Charlie Wilson")
};

int main() {
    crow::SimpleApp app;

    // Applicant Registration
    CROW_ROUTE(app, "/register").methods("POST"_method)([](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        string name = body["name"].s();
        float cgpa = stof(string(body["cgpa"].s()));
        string skills = body["skills"].s();
        string email = body["email"].s();

        applicants.emplace_back(name, cgpa, skills, email);
        return crow::response(200, "Application submitted successfully!");
    });

    // Employee Login
    CROW_ROUTE(app, "/employee-login").methods("POST"_method)([](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        string username = body["username"].s();
        string password = body["password"].s();

        for (auto& emp : employees) {
            if (emp.authenticate(username, password)) {
                return crow::response(200, "Welcome Employee: " + emp.getName());
            }
        }
        return crow::response(401, "Invalid credentials for Employee.");
    });

    // HR Login + View Applicants
    CROW_ROUTE(app, "/hr-login").methods("POST"_method)([](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        string username = body["username"].s();
        string password = body["password"].s();

        for (auto& hr : hrExecutives) {
            if (hr.authenticate(username, password)) {
                // Sort applicants by CGPA
                sort(applicants.begin(), applicants.end(), [](const Applicant& a, const Applicant& b){
                    return a.getCGPA() > b.getCGPA();
                });

                crow::json::wvalue result;
                int i = 0;
                for (auto& applicant : applicants) {
                    result["applicants"][i]["name"] = applicant.getName();
                    result["applicants"][i]["cgpa"] = applicant.getCGPA();
                    result["applicants"][i]["skills"] = applicant.getSkills();
                    result["applicants"][i]["email"] = applicant.getEmail();
                    result["applicants"][i]["status"] = applicant.getStatus();
                    i++;
                }
                return crow::response(result);
            }
        }
        return crow::response(401, "Invalid credentials for HR.");
    });

    app.port(18080).multithreaded().run();
}