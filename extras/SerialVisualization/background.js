chrome.app.runtime.onLaunched.addListener(function() {
    chrome.app.window.create('main.html', {
        frame: 'chrome',
        id: 'main-window',
        width: 1280,
        height: 720
    });
});