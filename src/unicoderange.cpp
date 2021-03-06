// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * TODO: insert short description here
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#include "unicoderange.h"

#include <cstdlib>
#include <cstring>

static unsigned int hex2int(char* s){
    int res=0;
    int i=0, mul=1;
    while(s[i+1]!='\0') i++;

    while(i>=0){
        if (s[i] >= 'A' && s[i] <= 'F') res += mul * (s[i]-'A'+10);
        if (s[i] >= 'a' && s[i] <= 'f') res += mul * (s[i]-'a'+10);
        if (s[i] >= '0' && s[i] <= '9') res += mul * (s[i]-'0');
        i--;
        mul*=16;
    }
    return res;
}

UnicodeRange::UnicodeRange(const gchar* value){
    if (!value) return;
    gchar* val = (gchar*) value;
    while(val[0] != '\0'){
        if (val[0]=='U' && val[1]=='+'){
            val += add_range(val+2);
        } else {
            this->unichars.push_back(g_utf8_get_char(&val[0]));
            val = g_utf8_next_char(val);
        }
        //skip spaces or commas
        while(val[0]==' ' || val[0]==',') val++;
    }
}

int UnicodeRange::add_range(gchar* val){
        Urange r;
        int i=0, count=0;
        while(val[i]!='\0' && val[i]!='-' && val[i]!=' ' && val[i]!=','){
            i++;
        }
        r.start = (gchar*) malloc((i+1)*sizeof(gchar));
        strncpy(r.start, val, i);
        r.start[i] = '\0';
        val+=i;
        count+=i;
        i=0;
        if (val[0]=='-'){
            val++;
            while(val[i]!='\0' && val[i]!='-' && val[i]!=' ' && val[i]!=',') i++;
            r.end = (gchar*) malloc((i+1)*sizeof(gchar));
            strncpy(r.end, val, i);
            r.end[i] = '\0';
            // val+=i;
            count+=i;
        } else {
            r.end=nullptr;
        }
        this->range.push_back(r);
        return count+1;
}

bool UnicodeRange::contains(gchar unicode){
    for(unsigned int unichar : this->unichars){
        if (static_cast<gunichar>(unicode) == unichar){
            return true;
        }
    }

    unsigned int unival;
    unival = g_utf8_get_char (&unicode);
    char uni[9] = "00000000";
    uni[8]= '\0';
    for (unsigned int i=7; unival>0; i--){
        unsigned char val = unival & 0xf;
        unival = unival >> 4;
        if (val < 10) uni[i] = '0' + val;
        else uni[i] = 'A'+ val - 10;
    }

    bool found;
    for(auto r : this->range){
        if (r.end){
            if (unival >= hex2int(r.start) && unival <= hex2int(r.end)) return true;
        } else {
            found = true;

            int p=0;
            while (r.start[p]!='\0') p++;
            p--;

            for (int pos=8;p>=0;pos--,p--){
                if (uni[pos]!='?' && uni[pos]!=r.start[p]) found = false;
            }
            if (found) return true;
        }
    }
    return false;
}

Glib::ustring UnicodeRange::attribute_string(){
    Glib::ustring result;
    unsigned int i;
    for(i=0; i<this->unichars.size(); i++){
g_warning("uni: %d %x", i, unichars[i]);
        result += this->unichars[i];
        if (i!=this->unichars.size()-1) result += ",";
    }

    for(i=0; i<this->range.size(); i++){
        result += "U+" + Glib::ustring(this->range[i].start);
        if (this->range[i].end) result += "-" + Glib::ustring(this->range[i].end);
        if (i!=this->range.size()-1) result += ", ";
    }

    return result;
}

gunichar UnicodeRange::sample_glyph(){
    //This could be better
    if (!unichars.empty())
        return unichars[0];
    if (!range.empty())
        return hex2int(range[0].start);
    return (gunichar) ' ';
}

