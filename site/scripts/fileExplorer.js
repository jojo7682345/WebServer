function getFilePath(){
    return document.getElementById('path').innerHTML;
}

function setFilePath(path){
    
    const pathElement = document.getElementById('path');
    pathElement.innerHTML = path;
}

function openFile(path, file){
    const fileName = document.getElementById('fileName');
    fileName.innerHTML = file.name;
    
    const fileContentPanel = document.getElementById('fileContent');        
    fileContentPanel.innerHTML = "loading...";

    if(file.fileType == "text"){
        fetch(`/open?path=${path}&file=${file.name}`,{
            method: 'GET'
        }).then(response => response.text()).then(text =>{
            
            fileContentPanel.innerHTML = `<pre>${text}</pre>`;
        } );
        return;
    }   
    if(file.fileType == "image"){
        fileContentPanel.innerHTML = `<img src="/open?path=${path}&file=${file.name}" width="100%"/>`;
        return;
    }
    if(file.fileType == "video"){
        fileContentPanel.innerHTML = `<video controls="true" playsinline autoplay><source src="/stream?path=${path}&file=${file.name}" type="video/mp4" /></video>`;
        return;
    }
    if(file.fileType == "audio"){
        fileContentPanel.innerHTML = `<audio src="/open?path=${path}&file=${file.name}" controls></audio>`;
        return;
    }
    if(file.fileType == "pdf"){
        fileContentPanel.innerHTML = `<iframe src="/open?path=${path}&file=${file.name}" width="100%" height="100%"></iframe>`;
        /*fileContentPanel.innerHTML = `<object data="/open?path=${path}&file=${file.name}" type="application/pdf">
                                        <div>No PDF viewer available</div>
                                        </object>`;*/
        return;
    }
    if(file.fileType == "url"){
        fetch(`/open?path=${path}&file=${file.name}`,{
            method: 'GET'
        }).then(response => response.text()).then(text =>{
            const lines = text.split("\n");
            if(lines[2]=="[InternetShortcut]\r"){
                var sourceUrl = lines[4].split("=")[1];
                sourceUrl = sourceUrl.replace("\r","");
                fileContentPanel.innerHTML = `<a href="${sourceUrl}" target="_blank">${sourceUrl}</a>`;
                return;
            }
            fileContentPanel.innerHTML = text;
            return;
        } );
        
        return;
    }
}

function dragOverHandler(event) {
    event.preventDefault();
    event.stopPropagation();
  }

function uploadFile(file, path){
    fetch(`/upload?path=${path}&file=${file.name}`, {
        method: 'POST',
        body: file.body
    }).then(response =>  {
        if(response.status == 200){
            listFilesInDirectory(getFilePath());
        }
        
    } );
}



function handleFileUpload(event,path){
    event.preventDefault();
    event.stopPropagation();
    
    if(!event.dataTransfer.items){
        [...event.dataTransfer.files].forEach((file, i) => {
           uploadFile(file, path);
        });
        return;
    }

    [...event.dataTransfer.items].forEach((item, i) => {
        // If dropped items aren't files, reject them
        if (item.kind === 'file') {
            let reader = new FileReader();
             const fileItem = item.getAsFile();
            reader.onload = (event) => {
                const file = {
                    name: fileItem.name,
                    body: event.target.result,
                    fileType: item.type
                };
                uploadFile(file, path);
            };
           
            reader.readAsBinaryString(fileItem);
          
          
        }
    });
}

function download(filename) {

    var element = document.createElement('a');
    element.setAttribute('href', '/download?path='+getFilePath()+'&file='+filename);
    element.setAttribute('download', filename);
    document.body.appendChild(element);
    element.click();
    document.body.removeChild(element);
}

function downloadFile(path, file){
    download(file, `download?path=${path}&file=${file}`);
}

function deleteFile(path, file){
    fetch(`/delete?path=${path}&file=${file}`,{
        method: 'POST',
        body: file
    }).then(response => {
        if(response.status == 200){
            openFile("/", {name: "/", isDirectory: true, fileType: "none"});
        }
        listFilesInDirectory(getFilePath());
    } );
}

function openOptionsMenu(fileElement, file, path){
    //fetch the possible options from the server
    fetch(`/options?path=${path}&file=${file.name}`).then(response => response.json()).then(options => {
        //create the options menu as a dropdown menu
        
        const fileContentPanel = document.getElementById('fileContent');
        const fileName = document.getElementById('fileName');

        fileContentPanel.innerHTML = " ";
        fileName.innerHTML = "Options of: " + file.name;

        const optionsMenu = document.createElement('ul');
        optionsMenu.classList.add('optionsMenu');

        const filePath = document.createElement('p');
        filePath.innerHTML = 'Path: ' + path;
        filePath.classList.add('filePath');
        optionsMenu.appendChild(filePath);

        for(let option of options){
            const optionElement = document.createElement('li');
            optionElement.classList.add('fileOption');
            optionElement.innerHTML = option;
            optionElement.addEventListener('click', function(){
                switch(option){
                    case "view":
                        if(file.isDirectory){
                            setFilePath(path + file.name + '/');
                            listFilesInDirectory(getFilePath());
                        }else{
                            openFile(path, file);
                        }
                    break;
                    case "rename":
                        
                    break;
                    case "download":
                        downloadFile(path, file.name);
                    break;
                    case "delete":
                        deleteFile(path, file.name);
                    break;
                }
            } );
            optionsMenu.appendChild(optionElement);
        };
        fileContentPanel.appendChild(optionsMenu);
      
    });

}

function generateFileElement(path, file){
    const listenEvent = 'click';
    
    let fileElement = document.createElement('li');
    const fileDiv = document.createElement('div');
    const labelDiv = document.createElement('div');
    const label = document.createElement('p');
    const options = document.createElement('p');

    fileElement.addEventListener("ondragover", dragOverHandler);

    if(file.isDirectory){
        fileElement.addEventListener("ondrop", function(event){
            handleFileUpload(event, path + file.name + '/');
        } );

        labelDiv.addEventListener(listenEvent, function(){
            setFilePath(path + file.name + '/');
            listFilesInDirectory(getFilePath());
        });
    }else{
        fileElement.addEventListener("ondrop", function(event){
            handleFileUpload(event, path);
        } );
        labelDiv.addEventListener(listenEvent, function(){
            openFile(getFilePath(), file);
        });
    }
    
    
    fileElement.classList.add('listedFile');

   
    fileDiv.classList.add('fileSplit');

    labelDiv.classList.add('fileNameLabelContainer');
    labelDiv.classList.add('left');
    
    fileDiv.appendChild(labelDiv);

    
    label.classList.add('fileNameLabel');
    label.innerHTML = file.name;
    labelDiv.appendChild(label);

    options.classList.add('fileOptions');
    options.classList.add('right');
    options.innerHTML = '···';
    options.addEventListener('click', function(){
        openOptionsMenu(fileElement, file, path);
    } );
    fileDiv.appendChild(options);

    fileElement.appendChild(fileDiv);

    return fileElement;
}

function listFilesInDirectory(path){

    fetch(`/list?path=${path}`).then(response => response.json()).then(files => {
        const fileList = document.getElementById('fileList');
        fileList.innerHTML = '';
        if(getFilePath() != '/'){
            const backButton = document.createElement('li');
            backButton.classList.add('listedFile');
            backButton.classList.add('directory');
            backButton.innerHTML = '<--';
            backButton.addEventListener('click', function(){
                const newFilePath = getFilePath().split('/').slice(0, -2).join('/');
                const dir = newFilePath+'/';
                setFilePath(dir);
                listFilesInDirectory(dir);
            });
            fileList.appendChild(backButton);
        }
        for(let file of files){
            fileList.appendChild(generateFileElement(path, file));
        }
    } );
}

listFilesInDirectory(getFilePath());
