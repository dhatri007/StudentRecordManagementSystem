# 🎓 Student Record Management System  
*A modern & elegant C++ + SFML GUI application for student data & attendance visualization.*

---

## 🌟 Overview

This system allows users to store, view, update, and visualize student records including academic details & attendance subject-wise.  
Designed with a **center-aligned responsive UI**, built using **C++ & SFML**, with a smooth & pastel interface for ease of user interaction.

This is fully GUI-based — no console work required for users.

---

## 🚀 Features

| Core Functionality | Status |
|---|---|
| Add new student details | ✔ Completed |
| Subject-wise attendance storing | ✔ Completed |
| Auto percentage calculation | ✔ Completed |
| Pie-chart visualization | ✔ Completed |
| Always centered UI 🔥 | ✔ Completed |
| Smooth Pastel Purple Interface | ✔ Completed |

---

## 🖥️ Screens Included

| Screen | Purpose |
|-------|----------|
| 🏠 Main Menu | Navigate system (Add / View / Attendance Chart / Exit) |
| ➕ Add Student | Enter student details + subjects + attendance |
| 🔍 View Details | Shows full student info by Roll No. |
| 📊 Attendance View | Shows subject-wise % table |
| 🥧 Pie Chart Screen | Visual subject distribution in multiple pleasant colors |

---

## 📊 Pie Chart Legend Example

| Color | Represents |
|------|------------|
| Lavender Purple 💜 | Subject 1 |
| Sky Blue 🔹 | Subject 2 |
| Mint Green 🍃 | Subject 3 |
| Soft Orange 🟧 | Subject 4 |
| Rose Pink 🌸 | Subject 5 |

(Same mapping appears on UI below the pie)

---

## 🏗 Technologies Used

| Component | Tech |
|---|---|
| GUI Framework | **SFML 3.0+** |
| Programming Language | **C++20** |
| Platform | Mac M1/M2 (primary support) |

---

## 🛠 Installation / Run Instructions

> Make sure SFML is installed  
> (`brew install sfml`)

Compile:

```bash
clang++ project.cpp -o app -std=c++20 \
-I/opt/homebrew/include -L/opt/homebrew/lib \
-lsfml-graphics -lsfml-window -lsfml-system
./app
