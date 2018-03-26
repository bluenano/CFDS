
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
    var formdata = new FormData();
    $scope.getTheFiles = function ($files) {
    angular.forEach($files, function (value, key) {
        formdata.append(key, value);
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
                console.log(request.responseText);
            }
        }

        request.open('POST', 'validate_video.php', true);
        request.send(formdata);
    }

});


//End of Upload Controller Content

app.controller('signUpCtrl', function($scope, $http) {

	$scope.createAccount = function ()
	{
		var username = $scope.username;
		var email = $scope.email;
		var firstname = $scope.firstname;
		var lastname = $scope.lastname;
		var password = $scope.password1;

		var dataArray = {username, email, firstname, lastname, password};
		    
		$.ajax({
	  		type:"POST",
	  		url: "create_account.php",
	  		data: dataArray,
	  		cache: false,
	  		success: function(data) {
	  		console.log(data);
	  		}
	  	});
	}
});




