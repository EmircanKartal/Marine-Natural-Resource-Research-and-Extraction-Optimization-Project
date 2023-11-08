# Marine-Natural-Resource-Research-and-Extraction-Optimization-Project
## Overview
This project, conducted at Kocaeli University's Computer Engineering Department, aims to optimize natural resource exploration and extraction operations in oceans. The goal is to divide the search area into accurate and optimal parcels to maximize profits for a company involved in these operations.
## Contributors
<div align="center">
  <div style="float: left; padding-right: 20px;">
    <a href="https://linkedin.com/in/emircankartal" target="blank"><img src="https://raw.githubusercontent.com/rahuldkjain/github-profile-readme-generator/master/src/images/icons/Social/linked-in-alt.svg" alt="emircankartal" height="27" width="36" /></a>
    <br />
    **Emircan Kartal**
    <br />
    <a href="mailto:emircan@example.com"><img src="https://raw.githubusercontent.com/rahuldkjain/github-profile-readme-generator/master/src/images/icons/Social/mail.svg" alt="Email" height="27" width="36" /></a>
  </div>
  <div style="float: right; padding-left: 20px;">
    <a href="https://www.linkedin.com/in/cagri-atalar-354692166/" target="blank"><img src="https://raw.githubusercontent.com/rahuldkjain/github-profile-readme-generator/master/src/images/icons/Social/linked-in-alt.svg" alt="cagriatalar" height="27" width="36" /></a>
    <br />
    **Çağrı Atalar**
    <br />
    <a href="mailto:cagri@example.com"><img src="https://raw.githubusercontent.com/rahuldkjain/github-profile-readme-generator/master/src/images/icons/Social/mail.svg" alt="Email" height="27" width="36" /></a>
  </div>
</div>




## Getting Started
### 1. Prerequisites
 - Ensure you have SDL (Simple DirectMedia Layer) library installed. You can find installation instructions here.
### 2. Installation
 - Clone the repository: git clone <repository-url>
 - Compile the project using a C/C++ compiler.
### 3. Usage
 - Run the compiled executable.
 - Input the necessary parameters when prompted (e.g., number of rows, unit drilling cost, unit platform cost).
## Project Structure
src/: Contains the source code files.
data/: Includes sample data files for testing the program.
images/: Provides visual representations for better understanding.

# How It Works
### 1. Data Input: The program reads coordinates from a text file via an internet connection. 
![data input](https://github.com/EmircanKartal/Marine-Natural-Resource-Research-and-Extraction-Optimization-Project/assets/88210656/4a3fb606-d008-4932-8a3e-78d867f2cfc4)
### 2. Processing: The coordinates are processed and converted into 2D closed area shapes.
### 3. Optimization: The areas are optimized by dividing them into square parcels, minimizing costs and maximizing profits.
### 4. Visualization: The optimized search areas are visually represented.

![Github photo](https://github.com/EmircanKartal/Marine-Natural-Resource-Research-and-Extraction-Optimization-Project/assets/88210656/a81539e4-3406-46ca-9801-e8e886c46431)

![github2](https://github.com/EmircanKartal/Marine-Natural-Resource-Research-and-Extraction-Optimization-Project/assets/88210656/82399c90-9f74-4024-b250-617a64a8c2a9)

# Parsing Algoritm
In this project, coordinates from the URL are parsed using a loop that processes the data based on the user's selected row. The coordinates are extracted and stored in the program's data structure (dataset.points[i]) using pre-defined x and y variables. The number of coordinates in the selected row is tracked using the dataset.numPoints variable, allowing for accurate processing and calculations.

![Algoritma son hali](https://github.com/EmircanKartal/Marine-Natural-Resource-Research-and-Extraction-Optimization-Project/assets/88210656/7047f60c-58a9-45d4-b7d8-b69ff4b0f4a8)

## License
This project is licensed under the MIT License.



