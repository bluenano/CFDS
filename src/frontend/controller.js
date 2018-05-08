
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
                        localStorage.setItem("userid", result.userid);
                        console.log(localStorage.getItem("userid")); 
                        $.ajax({
                            type:"GET",
                            url: "api/video/read_all",
                            data: {userid: localStorage.getItem("userid")},
                            cache:false,
                            success: function(videoJSON) {
                                
                                for (i = 0; i < videoJSON.length; i++) {
                                    localStorage.setItem(i, videoJSON[i]["videoid"]);
                                    console.log(localStorage.getItem(i));
                                }
                                //window.location.href = "/cs160/test/home.html";
                            }
                        });                       
                    }
                    else
                    {
                        alert('Error: username or password incorrect');
                    }
                }
            });
    }
});

app.controller('homeCtrl', function($scope) {
    $scope.upload = function() {
        window.location.href = "/cs160/test/upload.html";
    }

});

