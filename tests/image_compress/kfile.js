let project = new Project('Image-Compress');

await project.addProject(findKore());

project.addFile('sources/**');
project.addFile('deployment/**');
project.setDebugDir('deployment');

project.addIncludeDir('temp');
project.addFile('temp/kong.m');
project.addFile('temp/kong.metal');

project.flatten();

resolve(project);
