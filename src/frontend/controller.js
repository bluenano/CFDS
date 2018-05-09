
var app = angular.module('myApp', []);

app.directive('ngFiles', ['$parse', function ($parse) {

            function fn_link(scope, element, attrs) {
                var onChange = $parse(attrs.ngFiles);
                element.on('change', function (event) {
                    onChange(scope, { $files: event.target.files });
                });
            };

            return {
                link: fn_link
            }
        } ])

//Upload controller content 
app.controller('uploadCtrl', function($scope, $http) 
{

    var formdata;
    $scope.getTheFiles = function ($files) {
    angular.forEach($files, function (value, key) {
        if(value.size > 30000000)
        {
            alert("File Too Big");
            var upload = document.getElementById('upload');
            upload.innerHTML = '';
            angular.element(file1).val(null);
        
        }
        else
        {
            formdata = new FormData();
            formdata.append(key, value);
            var upload = document.getElementById('upload');
            upload.innerHTML = '<input type="button" value="Upload" />';
            
        }
    });
    };

    // NOW UPLOAD THE FILES.
    $scope.uploadFiles = function () 
    {
        for (var value of formdata.values()) {
            console.log(value); 
        }

        var request = new XMLHttpRequest();

        request.onreadystatechange = function() {
            if (request.readyState == XMLHttpRequest.DONE) {
                var result = JSON.parse(request.responseText);
                if (result.success == true) {
                    window.location.href = "/cs160/test/home.html";
                }
            }
        }

        request.open('POST', 'api/scripts/upload.php', true);
        request.send(formdata);
    }

});


//End of Upload Controller Content

app.controller('signUpCtrl', function($scope, $http) {

	$scope.createAccount = function ()
	{
		var username = $scope.username;
	
		var firstname = $scope.firstname;
		var lastname = $scope.lastname;
		var password = $scope.password1;

		var dataArray = {username, firstname, lastname, password};
		    
		$.ajax({
	  		type:"POST",
	  		url: "api/scripts/create_account.php",
	  		data: dataArray,
	  		cache: false,
	  		success: function(data) {
	  		   var result = JSON.parse(data);
               if(result.success == true)
               {
                    window.location.href = "/cs160/test/home.html";
               }
               else
               {
                    alert('Error: username already exist');
               }
	  		}
	  	});
	}
});


app.controller('loginCtrl', function($scope, $http) {
    $scope.login = function ()
    {
        var username = $scope.username;
        var password = $scope.password;

        var dataArray = {username, password};

        $.ajax({
                type:"POST",
                url: "api/scripts/login.php",
                data: dataArray,
                cache: false,
                success: function(data) 
                {
                    var result = JSON.parse(data);

                    if(result.success == true)
                    {
                        
                        sessionStorage.setItem("userid", result.userid); 
                       /*
                        $.ajax({
                            type:"GET",
                            url: "api/video/read_all",
                            data: {userid: sessionStorage.getItem("userid")},
                            cache:false,
                            success: function(videoJSON) {
                                
                                for (i = 0; i < videoJSON.length; i++) {
                                    sessionStorage.setItem(i.toString(), JSON.stringify(videoJSON[i]));
                                    
                                    //console.log(sessionStorage.getItem(i));
                                }
                                
                            }
                        });   
                        */
                        window.location.href = "/cs160/test/home.html";                    
                    }
                    else
                    {
                        alert('Error: username or password incorrect');
                    }
                }
            });
    }
});

app.controller('homeCtrl', function($scope) 
{   
    $.ajax({
        type:"GET",
        url: "api/video/read_all.php",
        data: {userid: sessionStorage.getItem("userid")},
        cache:false,
        success: function(videoJSON) 
        {

        for (i = 0; i < videoJSON.length && i < 10; i++) 
        {
            console.log(videoJSON[i]);
            var videoid = videoJSON[i]["videoid"];
            var title = videoJSON[i]["title"];
            var uploaddate = videoJSON[i]["uploaddate"];
            //console.log(videoid + " " + title + " " + uploaddate);
            sessionStorage.setItem((i+1).toString(), videoid.toString());
            var temp1 = title + ' ';
            var temp2 = uploaddate + ' ';
            var temp3 = videoid;

            var result = temp1 + " " + temp2;

            document.getElementById(i + 1).innerHTML = temp1;
            document.getElementById(i + 1.1).innerHTML = temp2;
            var num1 = i + 1.2;
            var num2 = i + 1.3;
            num1 = num1.toString();
            num2 = num2.toString();
            document.getElementById(i + 1.2).innerHTML = "<th><button id =" + num1 + " type ='button' class='btn btn-success' >Play</button>";
            document.getElementById(i + 1.3).innerHTML = "<button id=" + num2 + " type ='button' class='btn btn-danger' >Delete</button></th>";

            var videoINeedToPlay = i + 1; 
            var videoINeedToDelete = i + 1;

            console.log(videoINeedToPlay);
            console.log(videoINeedToDelete);

            document.getElementById(i + 1.2).addEventListener("click", function()
            {
                
                //Function to play
                $.ajax({
                    type:"GET",
                    url:"api/video/read.php",
                    data: {userid: videoINeedToPlay},
                    cache: false,
                    success: function(data) 
                    {
                        window.location.href = "/cs160/test/videoPlayer.html";
                    }
                });

            });
            document.getElementById(i + 1.3).addEventListener("click", function()
            {
            
                //Function to delete
                $.ajax({
                    type:"GET",
                    url:"api/video/delete.php",
                    data: {userid: videoINeedToDelete},
                    cache: false,
                    success: function(data) 
                    {
                        window.location.href = "/cs160/test/home.html";
                    }
                });

            });

        }
                                
        }

    }); 


    $scope.upload = function() {
        window.location.href = "/cs160/test/upload.html";
    }

    $scope.logout = function ()
    {
        $.ajax({
            type:"GET",
            url: "api/scripts/logout.php",
            data: {userid: sessionStorage.getItem("userid")},
            cache: false,
            success: function(data) {
                window.location.href = "/cs160/test/login.html";
            } 

            });

    }

});











