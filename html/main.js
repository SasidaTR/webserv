document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('sendPost')?.addEventListener('click', function() {
        fetch('/test.txt', {
            method: 'POST',
            headers: { 'Content-Type': 'text/plain' },
            body: 'Ceci est un test de POST'
        }).then(r => r.text()).then(console.log);
    });

    document.getElementById('deleteFile')?.addEventListener('click', function() {
        fetch('/test.txt', { method: 'DELETE' })
            .then(r => r.text())
            .then(console.log);
    });
});