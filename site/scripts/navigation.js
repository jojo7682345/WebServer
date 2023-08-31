
//get the content iframe
var content = document.getElementById('pageContent');

//get the navigation buttons
var homeNavButton = document.getElementById('home');
var filesNavButton = document.getElementById('files');
var controlNavButton = document.getElementById('control');

//add event listeners to the navigation buttons
function addEventListeners(){
    homeNavButton.addEventListener('click', function(){
        //fetches the home page and include the userSessionId with the cookie
        //by using GET

        fetch('/html/home.html').then(response => response.text()).then(html => {
            content.innerHTML  = html;

            //execute scripts in the html
            nodeScriptReplace(content);

            homeNavButton.classList.add('active');
            filesNavButton.classList.remove('active');
            controlNavButton.classList.remove('active');

        })
    } );
    filesNavButton.addEventListener('click', function(){
        fetch('/html/files.html').then(response => response.text()).then(html => {
            content.innerHTML  = html;

            //execute scripts in the html
            nodeScriptReplace(content);

            homeNavButton.classList.remove('active');
            filesNavButton.classList.add('active');
            controlNavButton.classList.remove('active');
        });
    } );
    controlNavButton.addEventListener('click', function(){
        fetch('/html/control.html').then(response => response.text()).then(html => {
            content.innerHTML  = html;

            //execute scripts in the html
            nodeScriptReplace(content);

            homeNavButton.classList.remove('active');
            filesNavButton.classList.remove('active');
            controlNavButton.classList.add('active');
        });
    } );
}

addEventListeners();
homeNavButton.classList.add('active');