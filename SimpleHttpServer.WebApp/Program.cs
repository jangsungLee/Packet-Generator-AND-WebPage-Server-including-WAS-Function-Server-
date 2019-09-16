// Copyright (C) 2016 by Barend Erasmus and donated to the public domain

using SimpleHttpServer;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace WebContainerServer
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Try 1");
            // Start WebContainer Server
            HttpServer httpServer = new HttpServer(3010, Routes.GET);
            Thread thread = new Thread(new ThreadStart(httpServer.Listen));
            thread.Start();

        }
    }
}
