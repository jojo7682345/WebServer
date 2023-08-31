
function buildLoggedInPanel(panel,username, user){
    const div = document.createElement('div');

    // add a label for the username and for the premission level
    const usernameLabel = document.createElement('p');
    usernameLabel.innerHTML = "Logged in as: " + username;
    div.appendChild(usernameLabel);
    const permissionLevelLabel = document.createElement('p');
    const permissionLevel = user.permisionLevel;
    permissionLevelLabel.innerHTML = "Permission level: " + permissionLevel;

    div.appendChild(permissionLevelLabel);  

    panel.appendChild(div);
}

function submitLoginData(){
    var username = document.getElementById("username").value;
    var password = document.getElementById("password").value;
    // hash the password
    var hash = hashText(password);
    // send the hash to the server
    fetch('/login', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            username: username,
            password: hash
        })
    }).then(response =>{
        if(response.status==200){
            const loginPanel = document.getElementById("loginPanel");
            loginPanel.innerHTML = "";
            
            //read the body of the response and get the premission level
            response.json().then(user => {
                buildLoggedInPanel(loginPanel, username, user);
            }).catch(error => {
                console.log(error);
            });

        }else{
            alert("Login failed");
        }
    }).catch(error => {
        console.log(error);
    });
}