function updateFileContent() {
	fetch('/cgi-bin/read.py')
		.then(r => r.text())
		.then(text => {
			document.getElementById('fileContent').textContent = text;
		})
		.catch(console.error);
}

document.addEventListener('DOMContentLoaded', function() {
	updateFileContent();

	document.getElementById('sendPost')?.addEventListener('click', function() {
		fetch('/cgi-bin/post.py', {
			method: 'POST',
			headers: { 'Content-Type': 'text/plain' },
			body: 'Ceci est le contenu du fichier test.txt'
		})
		.then(() => updateFileContent())
		.catch(console.error);
	});

	document.getElementById('deleteFile')?.addEventListener('click', function() {
		fetch('/cgi-bin/delete.py', { method: 'DELETE' })
			.then(() => updateFileContent())
			.catch(console.error);
	});
});
