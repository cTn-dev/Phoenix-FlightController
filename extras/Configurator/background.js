chrome.app.runtime.onLaunched.addListener(function() {
    chrome.app.window.create('main.html', {
        frame: 'chrome',
        id: 'main-window',
        minWidth: 960,
        maxWidth: 960,
        minHeight: 500,
        maxHeight: 500
    });
});