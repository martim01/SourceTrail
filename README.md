# SourceTrail
Code::Blocks plugin to interface with SourceTrail

## Introduction
This is a simple plugin for the Code::Blocks IDE that allows communication between Code::Blocks and SourceTrail.
Once installed in Code::Blocks it allows 
* The user to select a function/variable etc from Code::Blocks and ask SourceTrail to set it as the Active Token
* The user to select a token from SourceTrail and view the code in Code::Blocks.

## Building The Plugin
There is a Code::Blocks project which will build the plugin (currently only tested under Windows and with Code::Blocks 17.12).
In order to build the plugin 
* You need to download source code for Code::Blocks and wxWidgets for the headers.
* You need to set the following Code::Blocks variables
  * wx28.INCLUDE - to the location of wxWidgets headers
  * wx28.LIB - to the location of the wxWidgets library used by your installation of Code::Blocks
  * cb.INCLUDE - to the location of the Code::Blocks header files
  * cb.LIB - to the location of the Codeblocks library used by your installation of Code::Blocks

## Usage
* Firstly you need to import the Code::Blocks project you are interested in to SourceTrail
* To view the code in Code::Blocks of a token in SourceTrail simply click on the token whilst pressing CTRL
* To view a token in SourceTrail from Code::Blocks move the caret to the token you want to view and either
  * Right Click and select "SourceTrail: Show Token"
  * Click on the Edit menu and select "SourceTrail: Show Token"
  
## Communication
SourceTrail uses two local TCP connections to communicate with external editors. By default these two connections use
* port 6666 for outgoing messages
* port 6667 for incoming messages
It is possible to change these ports in SourceTrail but __not__ currently possible to change them in this plugin
