// Copyright (C) 2016 by Barend Erasmus and donated to the public domain

using SimpleHttpServer.Models;
using SimpleHttpServer.RouteHandlers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WebContainerServer
{
    static class Routes
    {

        public static List<Route> GET
        {
            get
            {
                return new List<Route>()
                {

                    new Route()
                    {
                        Callable = HomeIndex,
                        UrlRegex = "^\\/$",
                        Method = "GET"
                    },
                    new Route()
                    {
                        Callable = new FileSystemRouteHandler() { BasePath = @"C:\Users\Barend.Erasmus\Desktop\Test"}.Handle,
                        UrlRegex = "^\\/ANU\\/(.*)$",
                        Method = "GET"
                    }
                };

            }
        }

        private static HttpResponse HomeIndex(HttpRequest request)
        {
            var response = new HttpResponse();
            response.StatusCode = "200";
            response.ReasonPhrase = "Ok";
            response.Headers["Content-Type"] = "text/html";
            response.Content = Encoding.UTF8.GetBytes("<html lang=\"kr\" >" +
                "<head>" +
                "<meta charset=\"utf-8\">" +
                " </head>" +
                "<body>"+
                "<noscript>"+
                "<center>"+
                "<font color=\"red\">이 사이트를 이용하기 위해서는 자바스크립트를 활성화 시킬 필요가 있습니다.</font>"+
                "</center>"+
                "<a href=\"https://www.enable-javascript.com/ko/\">"+
                "</noscript>"+
                "</body>"+
                " <script type=\"text/javascript\">" +
                "window.onload=go_default;" +
                "function go_default() " +
                "{" +
                "  location.href=\"ANU/\\ethernet_frame.html\";" +
                "}" +
                "   </script>" +
                "  </html>");

            return response;

        }
    }
}
