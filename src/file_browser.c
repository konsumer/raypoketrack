#include "file_browser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char g_result[512] = {0};
static int  g_ready       = 0;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE void c_file_browser_ready(const char *path) {
    strncpy(g_result, path, sizeof(g_result) - 1);
    g_result[sizeof(g_result) - 1] = '\0';
    g_ready = 1;
}

EM_JS(void, js_file_open, (const char *filter_c), {
    var filterStr = UTF8ToString(filter_c);
    var accept = filterStr.split(' ').map(function(p) {
        return p.replace('*', '');
    }).join(',');
    var input = document.createElement('input');
    input.type = 'file';
    if (accept) input.accept = accept;
    input.style.display = 'none';
    input.onchange = function(e) {
        var file = e.target.files[0];
        if (!file) return;
        var reader = new FileReader();
        reader.onload = function(ev) {
            var data = new Uint8Array(ev.target.result);
            var path = '/uploads/' + file.name;
            try { FS.mkdir('/uploads'); } catch(err) { void err; }
            FS.writeFile(path, data);
            Module.ccall('c_file_browser_ready', null, ['string'], [path]);
        };
        reader.readAsArrayBuffer(file);
        document.body.removeChild(input);
    };
    document.body.appendChild(input);
    input.click();
});

EM_JS(void, js_file_download, (const char *fs_path_c, const char *name_c), {
    var path = UTF8ToString(fs_path_c);
    var name = UTF8ToString(name_c);
    try {
        var data = FS.readFile(path);
        var blob = new Blob([data], {type: 'application/octet-stream'});
        var url  = URL.createObjectURL(blob);
        var a    = document.createElement('a');
        a.href = url;
        a.download = name;
        a.style.display = 'none';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    } catch(err) { console.error('download:', err); }
});

void file_browser_open(const char *title, const char *filter) {
    (void)title;
    g_ready = 0;
    js_file_open(filter ? filter : "");
}

void file_browser_save_as(const char *title, const char *default_name) {
    (void)title;
    const char *p = default_name ? default_name : "song.rpt";
    strncpy(g_result, p, sizeof(g_result) - 1);
    g_result[sizeof(g_result) - 1] = '\0';
    g_ready = 1;
}

void file_browser_download(const char *fs_path, const char *suggested_name) {
    js_file_download(fs_path, suggested_name ? suggested_name : "download");
}

void file_browser_tick(void) {}

#else
#include "tinyfiledialogs.h"

static void parse_filter(const char *filter, const char **fptrs, char *fbuf, int bufsz, int *fnc) {
    *fnc = 0;
    if (!filter || !filter[0]) return;
    strncpy(fbuf, filter, bufsz - 1);
    char *tok = fbuf;
    while (*tok && *fnc < 15) {
        fptrs[(*fnc)++] = tok;
        tok = strchr(tok, ' ');
        if (!tok) break;
        *tok++ = '\0';
    }
}

void file_browser_open(const char *title, const char *filter) {
    const char *fptrs[16]; char fbuf[256] = {0}; int fnc = 0;
    parse_filter(filter, fptrs, fbuf, sizeof(fbuf), &fnc);
    const char *p = tinyfd_openFileDialog(title ? title : "Open", "", fnc, fptrs, NULL, 0);
    if (p) { strncpy(g_result, p, sizeof(g_result) - 1); g_ready = 1; }
    else    { g_ready = 0; }
}

void file_browser_save_as(const char *title, const char *default_name) {
    const char *fptrs[1] = {"*.rpt"};
    const char *p = tinyfd_saveFileDialog(
        title ? title : "Save", default_name ? default_name : "song.rpt", 1, fptrs, NULL);
    // If cancelled, p is NULL — do NOT set g_ready so caller skips the save
    if (p) { strncpy(g_result, p, sizeof(g_result) - 1); g_ready = 1; }
    else    { g_ready = 0; }
}

void file_browser_download(const char *fs_path, const char *suggested_name) {
    (void)fs_path; (void)suggested_name; // no-op on desktop
}

void file_browser_tick(void) {}
#endif

const char *file_browser_poll(void) {
    if (!g_ready) return NULL;
    g_ready = 0;
    return g_result;
}
