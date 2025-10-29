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

	let btn = document.getElementById('sendPost')
	btn.setAttribute('contenteditable', 'true')
	btn.textContent = 'POST'
	btn.addEventListener('keydown', function(e) {
		if(e.key === 'Enter') {
			e.preventDefault()
			fetch('/cgi-bin/post.py', {
				method: 'POST',
				headers: { 'Content-Type': 'text/plain' },
				body: btn.textContent
			})
			.then(() => updateFileContent())
			.catch(console.error)
			btn.textContent = ''
		}
	})

	document.getElementById('deleteFile')?.addEventListener('click', function() {
		fetch('/cgi-bin/delete.py', { method: 'DELETE' })
			.then(() => updateFileContent())
			.catch(console.error);
	});
});