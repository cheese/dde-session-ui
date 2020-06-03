#!/bin/bash

cp ".transifexrc" ${HOME}/

lupdate ./ -ts -no-obsolete translations/dde-session-ui.ts

tx push -s -b maintain/5.0_0101
