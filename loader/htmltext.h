#ifndef HTMLTEXT_H
#define HTMLTEXT_H

/**
 * @brief Plain text extractor for the 2 real HTML of the original APK that Android displayed in a WebView (about.html/help_eng.html, see).
 */
#define HTMLTEXT_STYLE_HEADER '\x01'
#define HTMLTEXT_STYLE_LABEL  '\x02'

/**
 * @brief Plain text extractor for the 2 real HTML of the original APK that Android displayed in a WebView (about.html/help_eng.html, see).
 */
int htmltext_extract(const char *html, char *out, int out_cap);

#endif
